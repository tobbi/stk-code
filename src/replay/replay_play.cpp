//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "replay/replay_play.hpp"

#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/controller/ghost_controller.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"

#include <irrlicht.h>
#include <stdio.h>
#include <string>

ReplayPlay::SortOrder ReplayPlay::m_sort_order = ReplayPlay::SO_DEFAULT;
ReplayPlay *ReplayPlay::m_replay_play = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
ReplayPlay::ReplayPlay()
{
    m_current_replay_file = 0;
}   // ReplayPlay

//-----------------------------------------------------------------------------
/** Frees all stored data. */
ReplayPlay::~ReplayPlay()
{
}   // ~Replay

//-----------------------------------------------------------------------------
/** Resets all ghost karts back to start position.
 */
void ReplayPlay::reset()
{
    for(unsigned int i=0; i<(unsigned int)m_ghost_karts.size(); i++)
    {
        m_ghost_karts[i].reset();
    }
}   // reset

//-----------------------------------------------------------------------------
void ReplayPlay::loadAllReplayFile()
{
    m_replay_file_list.clear();
    std::set<std::string> files;
    file_manager->listFiles(files, file_manager->getReplayDir(),
        /*is_full_path*/ false);

    char s[1024], s1[1024];
    for (std::set<std::string>::iterator i  = files.begin();
                                         i != files.end(); ++i)
    {
        if (StringUtils::getExtension(*i) != "replay") continue;
        FILE *fd = fopen((file_manager->getReplayDir() + (*i)).c_str(), "r");
        if (fd == NULL) continue;
        ReplayData rd;

        rd.m_filename = *i;

        fgets(s, 1023, fd);
        unsigned int version;
        if (sscanf(s,"version: %u", &version) != 1)
        {
            Log::warn("Replay", "No Version information "
                "found in replay file (bogus replay file).");
            fclose(fd);
            continue;
        }
        if (version != getReplayVersion())
        {
            Log::warn("Replay", "Replay is version '%d'", version);
            Log::warn("Replay", "STK version is '%d'", getReplayVersion());
            Log::warn("Replay", "Skipped '%s'", i->c_str());
            fclose(fd);
            continue;
        }

        while(true)
        {
            fgets(s, 1023, fd);
            core::stringc is_end(s);
            is_end.trim();
            if (is_end == "kart_list_end") break;
            char s1[1024];

            if (sscanf(s,"kart: %s", s1) != 1)
            {
                Log::warn("Replay", "Could not read ghost karts info!");
                break;
            }
            rd.m_kart_list.push_back(std::string(s1));
        }

        int reverse = 0;
        fgets(s, 1023, fd);
        if(sscanf(s, "reverse: %d", &reverse) != 1)
        {
            Log::warn("Replay", "Reverse info found in replay file.");
            fclose(fd);
            continue;
        }
        rd.m_reverse = reverse!=0;

        fgets(s, 1023, fd);
        if (sscanf(s, "difficulty: %u", &rd.m_difficulty) != 1)
        {
            Log::warn("Replay", " No difficulty found in replay file.");
            fclose(fd);
            continue;
        }

        fgets(s, 1023, fd);
        if (sscanf(s, "track: %s", s1) != 1)
        {
            Log::warn("Replay", "Track info not found in replay file.");
            fclose(fd);
            continue;
        }
        rd.m_track_name = std::string(s1);
        Track* t = track_manager->getTrack(rd.m_track_name);
        if (t == NULL)
        {
            Log::warn("Replay", "Track '%s' used in replay not found in STK!",
                rd.m_track_name.c_str());
            fclose(fd);
            continue;
        }

        fgets(s, 1023, fd);
        if (sscanf(s, "laps: %u", &rd.m_laps) != 1)
        {
            Log::warn("Replay", "No number of laps found in replay file.");
            fclose(fd);
            continue;
        }

        fgets(s, 1023, fd);
        if (sscanf(s, "min_time: %f", &rd.m_min_time) != 1)
        {
            Log::warn("Replay", "Finish time not found in replay file.");
            fclose(fd);
            continue;
        }
        fclose(fd);
        m_replay_file_list.push_back(rd);
    }
}   // loadAllReplayFile

//-----------------------------------------------------------------------------
void ReplayPlay::load()
{
    m_ghost_karts.clearAndDeleteAll();
    char s[1024];

    FILE *fd = openReplayFile(/*writeable*/false);
    if(!fd)
    {
        Log::error("Replay", "Can't read '%s', ghost replay disabled.",
               getReplayFilename().c_str());
        destroy();
        return;
    }

    Log::info("Replay", "Reading replay file '%s'.", getReplayFilename().c_str());

    const unsigned int line_skipped = getNumGhostKart() + 7;
    for (unsigned int i = 0; i < line_skipped; i++)
        fgets(s, 1023, fd);

    // eof actually doesn't trigger here, since it requires first to try
    // reading behind eof, but still it's clearer this way.
    while(!feof(fd))
    {
        if(fgets(s, 1023, fd)==NULL)  // eof reached
            break;
        readKartData(fd, s);
    }

    fprintf(fd, "Replay file end.\n");
    fclose(fd);
}   // load

//-----------------------------------------------------------------------------
/** Reads all data from a replay file for a specific kart.
 *  \param fd The file descriptor from which to read.
 */
void ReplayPlay::readKartData(FILE *fd, char *next_line)
{
    char s[1024];
    const unsigned int kart_num = m_ghost_karts.size();
    m_ghost_karts.push_back(new GhostKart(m_replay_file_list
        [m_current_replay_file].m_kart_list.at(kart_num),
        kart_num, kart_num + 1));
    m_ghost_karts[kart_num].init(RaceManager::KT_GHOST);
    Controller* controller = new GhostController(getGhostKart(kart_num));
    getGhostKart(kart_num)->setController(controller);

    unsigned int size;
    if(sscanf(next_line,"size: %u",&size)!=1)
        Log::fatal("Replay", "Number of records not found in replay file "
            "for kart %d.", kart_num);

    for(unsigned int i=0; i<size; i++)
    {
        fgets(s, 1023, fd);
        float x, y, z, rx, ry, rz, rw, time, speed, steer, w1, w2, w3, w4;
        int nitro, zipper, jumping;

        // Check for EV_TRANSFORM event:
        // -----------------------------
        if(sscanf(s, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f  %d %d %d\n",
            &time,
            &x, &y, &z,
            &rx, &ry, &rz, &rw,
            &speed, &steer, &w1, &w2, &w3, &w4,
            &nitro, &zipper, &jumping
            )==17)
        {
            btQuaternion q(rx, ry, rz, rw);
            btVector3 xyz(x, y, z);
            PhysicInfo pi = {0};
            KartReplayEvent kre = {0};
            pi.m_speed = speed;
            pi.m_steer = steer;
            pi.m_suspension_length[0] = w1;
            pi.m_suspension_length[1] = w2;
            pi.m_suspension_length[2] = w3;
            pi.m_suspension_length[3] = w4;
            kre.m_on_nitro = nitro!=0;
            kre.m_on_zipper = zipper!=0;
            kre.m_jumping = jumping!=0;
            m_ghost_karts[kart_num].addReplayEvent(time,
                btTransform(q, xyz), pi, kre);
        }
        else
        {
            // Invalid record found
            // ---------------------
            Log::warn("Replay", "Can't read replay data line %d:", i);
            Log::warn("Replay", "%s", s);
            Log::warn("Replay", "Ignored.");
        }
    }   // for i

}   // readKartData

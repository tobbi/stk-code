//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_GHOST_REPLAY_INFO_DIALOG_HPP
#define HEADER_GHOST_REPLAY_INFO_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "replay/replay_play.hpp"

/** \brief Dialog that allows a user to do action with ghost replay file
 * \ingroup states_screens
 */
class GhostReplayInfoDialog : public GUIEngine::ModalDialog
{

private:

    bool m_self_destroy;

    const unsigned int m_replay_id;
    ReplayPlay::ReplayData m_rd;

    GUIEngine::RibbonWidget* m_action_widget;
    GUIEngine::IconButtonWidget* m_back_widget;

public:
    GhostReplayInfoDialog(unsigned int replay_id);
    ~GhostReplayInfoDialog();

    GUIEngine::EventPropagation processEvent(const std::string& eventSource);

    virtual bool onEscapePressed();
    virtual void onUpdate(float dt);
};   // class GhostReplayInfoDialog

#endif

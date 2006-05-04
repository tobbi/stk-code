//  $Id: KartProperties.cxx,v 1.5 2005/08/10 07:58:10 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <iostream>
#include <stdexcept>
#include <plib/ssg.h>
#include "MaterialManager.h"
#include "lisp/Parser.h"
#include "lisp/Lisp.h"
#include "Loader.h"
#include "preprocessor.h"
#include "StringUtils.h"
#include "KartProperties.h"
#include "PhysicsParameters.h"

// This constructor would be a bit more useful, nicer, if we could call 
// init_defaults() and load from here. Unfortunately, this object is used
// as a base class for PhysicsParameters, which has to overwrite
// init_defaults() and getAllData(). But during the call of this constructor,
// the PhysicsParameters object does not (yet) exist, so the overwriting
// functions do NOT get called, only the virtual functions here would be
// called. Therefore, a two step initialisation is necessary: the constructor
// doing not much, but then in load the overwriting functions can be used.
KartProperties::KartProperties() : icon_material(0), model(0) {
}   // KartProperties

// -----------------------------------------------------------------------------
void KartProperties::load(const std::string& filename, char *node) {

  init_defaults();

  const lisp::Lisp* root = 0;
  ident = StringUtils::basename(StringUtils::without_extension(filename));

  try {
    lisp::Parser parser;
    root = parser.parse(loader->getPath(filename));
    
    const lisp::Lisp* lisp = root->getLisp(node);
    if(!lisp) {
      std::string s="No '";
      s+=node;
      s+="' node found";
      throw std::runtime_error(s);
    }
    getAllData(lisp);
  } catch(std::exception& err) {
    std::cout << "Error while parsing KartProperties '" << filename
              << ": " << err.what() << "\n";
  }
  delete root;

  // Load material
  icon_material = material_manager->getMaterial(icon_file.c_str());

  // Load model
  if(model_file.length()>0) {
    //JH    model         = ssgLoadAC ( ("models/"+model_file).c_str(), loader ) ;
    model         = ssgLoadAC ( model_file.c_str(), loader ) ;
    preProcessObj(model, 0);
    model->ref();
  }

}   // load

// -----------------------------------------------------------------------------
KartProperties::~KartProperties() {
  ssgDeRefDelete(model);
}   // ~KartProperties

// -----------------------------------------------------------------------------
void KartProperties::getAllData(const lisp::Lisp* lisp) {
    lisp->get("name",            name);
    lisp->get("model-file",      model_file);
    lisp->get("icon-file",       icon_file);
    lisp->get("shadow-file",     shadow_file);
    lisp->get("red",             color[0]);
    lisp->get("green",           color[1]);
    lisp->get("blue",            color[2]);

    lisp->get("wheel-base",      wheel_base);
    lisp->get("heightCOG",       heightCOG);
    lisp->get("engine-power",    engine_power);
    lisp->get("roll-resistance", roll_resistance); 
    lisp->get("mass",            mass);
    lisp->get("air-friction",    air_friction);
    lisp->get("tire-grip",       tire_grip);
    lisp->get("max-steer-angle", max_steer_angle);

    lisp->get("corn-f",          corn_f);
    lisp->get("corn-r",          corn_r);
    lisp->get("inertia",         inertia);
}   // getAllData
// -----------------------------------------------------------------------------
void KartProperties::init_defaults() {

  name          = "Tux";
  ident         = "tux";
  model_file    = "tuxkart.ac";
  icon_file     = "tuxicon.png";
  shadow_file   = "tuxkartshadow.png";

  color[0] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;

  wheel_base      = physicsParameters->wheel_base;
  heightCOG       = physicsParameters->heightCOG;
  engine_power    = physicsParameters->engine_power;
  roll_resistance = physicsParameters->roll_resistance;
  mass            = physicsParameters->mass;
  air_friction    = physicsParameters->air_friction;
  tire_grip       = physicsParameters->tire_grip;
  max_steer_angle = physicsParameters->max_steer_angle;


  corn_f          = physicsParameters->corn_f;
  corn_r          = physicsParameters->corn_r;
  inertia         = physicsParameters->inertia;

}   // init_defaults

// -----------------------------------------------------------------------------
/* EOF */

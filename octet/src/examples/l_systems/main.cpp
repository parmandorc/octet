////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// L-systems - tree generator
//

#include "../../octet.h"

#include "l_systems.h"

/// Create a box with octet
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::l_systems app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}



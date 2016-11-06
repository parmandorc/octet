////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.

  //Collision callback function
  /* PLEASE NOTE:
   * There has been severe problems with the use of the user pointer in the callback function.
   * Whenever I set the user pointer, fired balls stop updating its render (they would appear
   *  to stay in their initial position, but their position is actually being updated, as they
   *  would collide with the bridges and make them move).
   * I have tried using a struct too, but the problem remains.
   * I have also tried using the function setUserIndex, but the game would completely crash.
   * For all these problems, I have decided to leave the code so that I can demonstrate I have the knowledge
   *  on how to manage callback functions, but comment it so the game keeps working or doesn't crash.
   */
  bool contactCallbackFunc(btManifoldPoint& cp,
    const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
    const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {

    //mesh_instance *ball = (mesh_instance*)colObj0Wrap->getCollisionObject()->getUserPointer();
    //ball->set_material(new material(vec4(1.0f, 0.0f, 0.0f, 1.0f)));
    return false;
  }

  class example_shapes : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    //Main camera
    camera_instance *main_camera;
    float cam_vert_rot = 0.0f;

    //Mouse position
    float prev_mx = 0.0f, prev_my = 0.0f;
    bool was_mouse_down = false;
    unsigned int since_mouse_down = 0;

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
    }

    ~example_shapes() {
    }

    void create_spring(btRigidBody *rbA, btRigidBody *rbB, float length, float frame_offset_x, 
        float stiffness, float damping, 
        float frame_offset_z = 0.0f, bool force_stabilize = false) {

      btTransform frameInA = btTransform::getIdentity();
      frameInA.setOrigin(get_btVector3(vec3(frame_offset_x, 0.0f, frame_offset_z)));
      btTransform frameInB = btTransform::getIdentity();
      frameInB.setOrigin(get_btVector3(vec3(-frame_offset_x, 0.0f, frame_offset_z)));

      btGeneric6DofSpringConstraint *spring;
      if (!rbA || !rbB)
        spring = new btGeneric6DofSpringConstraint(rbA ? *rbA : *rbB, rbA ? frameInA : frameInB, true);
      else
        spring = new btGeneric6DofSpringConstraint(*rbA, *rbB, frameInA, frameInB, true);

      spring->setLinearLowerLimit(get_btVector3(vec3(0.0f, 0.0f, 0.0f)));
      spring->setLinearUpperLimit(get_btVector3(vec3(length, 0.0f, 0.0f)));
      for (int i = 0; i < (force_stabilize ? 6 : 1); i++) {
        spring->enableSpring(i, true);
        spring->setStiffness(i, stiffness);
        spring->setDamping(i, damping);
      }
      spring->setEquilibriumPoint(0, 0);
      app_scene->add_constraint(spring);
    }

    void create_hinge_bridge(mat4t mat, float bridge_length, float bridge_width, float plank_half_length, vec4 color, 
        float curvature_factor = 1.0f, bool force_stabilize = true) {

      material *colormat = new material(color);
      int number_of_planks = int(curvature_factor * bridge_length / (plank_half_length * 2.0f));
      float offset_bewteen_planks = plank_half_length * 2.0f + (bridge_length - plank_half_length * 2.0f * number_of_planks) / (number_of_planks - 1);

      //First plank
      mat.translate(- bridge_length * 0.5f + plank_half_length, 0, 0);
      mesh_instance *first_plank = app_scene->add_shape(mat, new mesh_box(vec3(plank_half_length, 0.2f, bridge_width * 0.5f)), colormat, true);
      btHingeConstraint *hinge = new btHingeConstraint(
        *first_plank->get_node()->get_rigid_body(), 
        get_btVector3(vec3(- plank_half_length, 0, 0)),
        get_btVector3(vec3(0, 0, 1))
      );
      app_scene->add_constraint(hinge);
      mat.translate(offset_bewteen_planks, 0, 0);

      //Planks loop
      mesh_instance *prev_plank = first_plank;
      for (int i = 1; i < number_of_planks; i++) {
        mesh_instance *plank = app_scene->add_shape(mat, new mesh_box(vec3(plank_half_length, 0.2f, bridge_width * 0.5f)), colormat, true);
        hinge = new btHingeConstraint(
          *prev_plank->get_node()->get_rigid_body(),
          *plank->get_node()->get_rigid_body(),
          get_btVector3(vec3(plank_half_length, 0, 0)),
          get_btVector3(vec3(- plank_half_length, 0, 0)),
          get_btVector3(vec3(0, 0, 1)),
          get_btVector3(vec3(0, 0, 1))
        );
        if (force_stabilize)
          hinge->setLimit(0, 0);
        app_scene->add_constraint(hinge);
        mat.translate(offset_bewteen_planks, 0, 0);
        prev_plank = plank;
      }

      //Last plank
      hinge = new btHingeConstraint(
        *prev_plank->get_node()->get_rigid_body(),
        get_btVector3(vec3(plank_half_length, 0, 0)),
        get_btVector3(vec3(0, 0, 1))
      );
      app_scene->add_constraint(hinge);
    }

    void create_spring_bridge(mat4t mat, float bridge_length, float bridge_width, float plank_half_length, vec4 color,
        float curvature_factor = 1.0f, float spring_stiffness = 80.0f, float spring_damping = 0.005f, bool force_stabilize = false) {

      material *colormat = new material(color);
      int number_of_planks = int(curvature_factor * bridge_length / (plank_half_length * 2.0f));
      float offset_bewteen_planks = plank_half_length * 2.0f + (bridge_length - plank_half_length * 2.0f * number_of_planks) / (number_of_planks - 1);
      float spring_max_length = 50 / spring_stiffness + 0.5f;

      //First plank
      mat.translate(- bridge_length * 0.5f + plank_half_length, 0, 0);
      mesh_instance *first_plank = app_scene->add_shape(mat, new mesh_box(vec3(plank_half_length, 0.2f, bridge_width * 0.5f)), colormat, true);
      create_spring(NULL, first_plank->get_node()->get_rigid_body(), spring_max_length, plank_half_length, spring_stiffness, spring_damping, 4.0f, force_stabilize);
      create_spring(NULL, first_plank->get_node()->get_rigid_body(), spring_max_length, plank_half_length, spring_stiffness, spring_damping, -4.0f, force_stabilize);
      mat.translate(offset_bewteen_planks, 0, 0);

      //Planks loop
      mesh_instance *prev_plank = first_plank;
      for (int i = 1; i < number_of_planks; i++) {
        mesh_instance *plank = app_scene->add_shape(mat, new mesh_box(vec3(plank_half_length, 0.2f, bridge_width * 0.5f)), colormat, true);
        create_spring(prev_plank->get_node()->get_rigid_body(), plank->get_node()->get_rigid_body(), spring_max_length, plank_half_length, spring_stiffness, spring_damping, 4.0f, force_stabilize);
        create_spring(prev_plank->get_node()->get_rigid_body(), plank->get_node()->get_rigid_body(), spring_max_length, plank_half_length, spring_stiffness, spring_damping, -4.0f, force_stabilize);
        mat.translate(offset_bewteen_planks, 0, 0);
        prev_plank = plank;
      }

      //Last plank
      create_spring(prev_plank->get_node()->get_rigid_body(), NULL, spring_max_length, plank_half_length, spring_stiffness, spring_damping, 4.0f, force_stabilize);
      create_spring(prev_plank->get_node()->get_rigid_body(), NULL, spring_max_length, plank_half_length, spring_stiffness, spring_damping, -4.0f, force_stabilize);
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      main_camera = app_scene->get_camera_instance(0);
      main_camera->get_node()->translate(vec3(0, 4, 0));
      main_camera->set_far_plane(200.0f);
      
      build_scene();

      //Set collision callback function
      gContactAddedCallback = contactCallbackFunc;
    }

    // reads a csv file to build the scene
    void build_scene() {
      mat4t mat;
      std::ifstream is("./scene.csv");
      if (!is.good()) {
        is.close(); return;
      }

      // store the line here
      char buffer[2048];

      // loop over lines
      while (!is.eof()) {
        is.getline(buffer, sizeof(buffer));

        // loop over columns
        char *b = buffer;
        
        if (*b == '#') continue; //skip commented line

        // definition data for the object
        enum {
          BOX,
          HINGE_BRIDGE,
          SPRING_BRIDGE
        } type;
        std::vector<float> params;

        // Read CSV line
        for (int col = 0; ; ++col) {
          while (*b == ' ' || *b == '\t') ++b;
          char *e = b;
          while (*e != 0 && *e != ',' && *e != ' ' && *e != '\t') ++e;

          // now b -> e contains the chars in a column
          if (col == 0) {
            std::string type_s = std::string(b, e);
            if (!type_s.compare("box"))
              type = BOX;
            else if (!type_s.compare("hinge_bridge"))
              type = HINGE_BRIDGE;
            else if (!type_s.compare("spring_bridge"))
              type = SPRING_BRIDGE;
            else
              break;
          }
          else
            params.push_back(std::atof(b));

          while (*e == ' ' || *e == '\t') ++e;
          if (*e != ',') break;
          b = e + 1;
        }

        //Build the object in the scene
        mat.loadIdentity();

        switch (type) {
        case BOX:
        {
          if (params.size() >= 6) {
            float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
            if (params.size() >= 9) {
              r = params[6];
              g = params[7];
              b = params[8]; 
              if (params.size() >= 10)
                a = params[9];
            }
            mat.translate(vec3(params[0], params[1], params[2]));
            app_scene->add_shape(mat, new mesh_box(vec3(params[3] * 0.5f, params[4] * 0.5f, params[5] * 0.5f)), new material(vec4(r, g, b, a)), false);
          }
        }
        break;

        case HINGE_BRIDGE:
        {
          if (params.size() > 6) {
            float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
            float curvature_factor = 1.0f;
            bool force_stabilize = false;
            if (params.size() >= 9) {
              r = params[6];
              g = params[7];
              b = params[8];
              if (params.size() >= 10) {
                a = params[9];
                if (params.size() >= 11) {
                  curvature_factor = params[10];
                  if (params.size() >= 12) {
                    force_stabilize = params[11] != 0;
                  }
                }
              }
            }
            mat.translate(vec3(params[0], params[1], params[2]));
            create_hinge_bridge(mat, params[3], params[4], params[5] * 0.5f, vec4(r, g, b, a), curvature_factor, force_stabilize);
          }
        }
        break;

        case SPRING_BRIDGE:
        {
          if (params.size() > 6) {
            float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
            float curvature_factor = 1.0f, stiffness = 80.0f, damping = 0.005f;
            bool force_stabilize = false;
            if (params.size() >= 9) {
              r = params[6];
              g = params[7];
              b = params[8];
              if (params.size() >= 10) {
                a = params[9];
                if (params.size() >= 11) {
                  curvature_factor = params[10];
                  if (params.size() >= 12) {
                    stiffness = params[11];
                    if (params.size() >= 13) {
                      damping = params[12];
                      if (params.size() >= 14) {
                        force_stabilize = params[13] != 0;
                      }
                    }
                  }
                }
              }
            }
            mat.translate(vec3(params[0], params[1], params[2]));
            create_spring_bridge(mat, params[3], params[4], params[5] * 0.5f, vec4(r, g, b, a), 
              curvature_factor, stiffness, damping, force_stabilize);
          }
        }
        break;

        }
      }

      is.close();
    }

    //controls player input to move the camera
    void move_camera() {
      //Keyboard input
      float dx = (is_key_down(key_right) || is_key_down('D')) - (is_key_down(key_left) || is_key_down('A'));
      float dy = is_key_down(key_space) - is_key_down(key_shift);
      float dz = - (is_key_down(key_up) || is_key_down('W')) + (is_key_down(key_down) || is_key_down('S'));
      
      //Process mouse movement (copied and modified from mouse_ball.h)
      float mdx = 0.0f, mdy = 0.0f;
      float sensitivity = 50.0f;
      bool is_mouse_down = is_key_down(key_rmb);
      if (is_mouse_down) {
        int mx = 0, my = 0;
        int vx = 0, vy = 0;
        get_mouse_pos(mx, my);
        get_viewport_size(vx, vy);
        if (was_mouse_down && vx && vy) {
          float cx = vx * 0.5f;
          float cy = vy * 0.5f;
          float pfx = (prev_mx - cx) / vx;
          float pfy = (prev_my - cy) / vy;
          float fx = (mx - cx) / vx;
          float fy = (my - cy) / vy;
          mdx = (pfx - fx) * sensitivity;
          mdy = (pfy - fy) * sensitivity;
        }
        prev_mx = mx;
        prev_my = my;
      }
      was_mouse_down = is_mouse_down;

      //Move camera
      main_camera->get_node()->rotate(-cam_vert_rot, vec3(1, 0, 0));
      main_camera->get_node()->rotate(mdx, vec3(0, 1, 0));
      main_camera->get_node()->translate(vec3(dx, dy, dz) * 0.5f);
      if (math::abs(cam_vert_rot + mdy) > 85.0f) { //Lock vertical rotation inside [-85,85]
        mdy = (85.0f - math::abs(cam_vert_rot)) * (math::abs(cam_vert_rot + mdy) >= 0.0f ? 1.0f : -1.0f);
      }
      cam_vert_rot += mdy;
      main_camera->get_node()->rotate(cam_vert_rot, vec3(1, 0, 0));
    }

    void fire() {
      // spawn new sphere
      if (is_key_down(key_lmb))
        ++since_mouse_down;

      if (is_key_going_up(key_lmb)) {
        mat4t mat;
        mat = main_camera->get_node()->calcModelToWorld();
        mat.translate(0, -2, -2);
        mesh_instance *newSphere = app_scene->add_shape(mat, new mesh_sphere(vec3(2, 2, 2), 1), new material(vec4(0.75f, 0.75f, 0.75f, 1)), true, 1.0f + since_mouse_down * 0.05f);
        newSphere->get_node()->apply_central_force(-mat.z() * 1500.0f * (1.0f + since_mouse_down * 0.1f));
        //newSphere->get_node()->get_rigid_body()->setUserPointer(newSphere);
        newSphere->get_node()->get_rigid_body()->setCollisionFlags(newSphere->get_node()->get_rigid_body()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

        since_mouse_down = 0;
      }
    }

    //update for game logic
    void simulate() {
      move_camera();

      fire();
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      simulate();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}

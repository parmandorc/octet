////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_shapes : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    //Main camera
    camera_instance *main_camera;
    float cam_vert_rot = 0.0f;

    //Mouse position
    float prev_mx = 0.0f, prev_my = 0.0f;
    bool was_mouse_down = false;

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
    }

    ~example_shapes() {
    }

	  int counter = 0;
	
	  class random randomizer;

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

      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

      float river_width = 20.0f;
      mat4t mat;
      mat.loadIdentity();
      mat.translate(vec3(- river_width * 0.5 - 2.0f, 5, 0));
      app_scene->add_shape(mat, new mesh_box(vec3(2, 5, 50)), green, false);
      mat.translate(vec3(river_width + 4.0f, 0, 0));
      app_scene->add_shape(mat, new mesh_box(vec3(2, 5, 50)), green, false);

      //front bridge
      mat.loadIdentity();
      mat.translate(0, 10, 6);
      create_spring_bridge(mat, river_width, 10.0f, 1.0f, vec4(0.55f, 0.25f, 0.1f, 1), 0.8f);

      //back bridge
      mat.loadIdentity();
      mat.translate(0, 10, -6);
      create_hinge_bridge(mat, river_width, 10.0f, 1.0f, vec4(0.55f, 0.25f, 0.1f, 1));

      // ground
      mat.loadIdentity();
      mat.translate(0, -10, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), blue, false);
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

    //update for game logic
    void simulate() {
      move_camera();

      // spawn new sphere when the counter reaches zero.
      if (counter-- <= 0) {
        counter = randomizer.get(10, 80);

        mat4t mat;
        mat.loadIdentity();
        mat.translate(0, 20, 0);
        mesh_instance *newSphere = app_scene->add_shape(mat, new mesh_sphere(vec3(2, 2, 2), 1), new material(vec4(0.75f, 0.75f, 0.75f, 1)), true);
        newSphere->get_node()->apply_central_force(vec3(randomizer.get(-200.0f, 200.0f), 0.0f, randomizer.get(-100.0f, 100.0f)));
      }
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

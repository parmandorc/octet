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

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
    }

    ~example_shapes() {
    }

	  int counter = 0;
	
	  class random randomizer;

    void create_spring(btRigidBody *rbA, btRigidBody *rbB, float length, float plankHalfLength, float stiffness, float damping, float offset = 0.0f) {
      btTransform frameInA = btTransform::getIdentity();
      frameInA.setOrigin(get_btVector3(vec3(plankHalfLength, 0.0f, offset)));
      btTransform frameInB = btTransform::getIdentity();
      frameInB.setOrigin(get_btVector3(vec3(-plankHalfLength, 0.0f, offset)));

      btGeneric6DofSpringConstraint *spring;
      if (!rbA || !rbB)
        spring = new btGeneric6DofSpringConstraint(rbA ? *rbA : *rbB, rbA ? frameInA : frameInB, true);
      else
        spring = new btGeneric6DofSpringConstraint(*rbA, *rbB, frameInA, frameInB, true);

      spring->setLinearLowerLimit(get_btVector3(vec3(0.0f, 0.0f, 0.0f)));
      spring->setLinearUpperLimit(get_btVector3(vec3(length * (rbA ? 1 : -1), 0.0f, 0.0f)));
      for (int i = 0; i < 6; i++) {
        spring->enableSpring(i, true);
        spring->setStiffness(i, stiffness);
        spring->setDamping(i, damping);
      }
      spring->setEquilibriumPoint(0, length * 0.5f * (rbA ? 1 : -1));
      app_scene->add_constraint(spring);
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 4, 0));

      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

      mat4t mat;

      //Bridge parameters
      float plankHalfLength = 1.0f;
      int numberOfPlanks = 8;
      float offsetBewteenPlanks = plankHalfLength * 2.0f + (20.0f - plankHalfLength * 2.0f * numberOfPlanks) / (numberOfPlanks - 1);
      float springLength = 1.0f;
      float springStiffness = 75.0f;
      float springDamping = 0.025f;

      //First plank
      mat.loadIdentity();
      mat.translate(-10.0f + plankHalfLength, 10, 0);
      mesh_instance *first_plank = app_scene->add_shape(mat, new mesh_box(vec3(plankHalfLength, 0.2f, 5)), red, true);
      create_spring(NULL, first_plank->get_node()->get_rigid_body(), springLength, plankHalfLength, springStiffness, springDamping, 4.0f);
      create_spring(NULL, first_plank->get_node()->get_rigid_body(), springLength, plankHalfLength, springStiffness, springDamping, -4.0f);
      mat.translate(offsetBewteenPlanks, 0, 0);

      //Planks loop
      mesh_instance *prev_plank = first_plank;
      for (int i = 1; i < numberOfPlanks; i++) {
        mesh_instance *plank = app_scene->add_shape(mat, new mesh_box(vec3(plankHalfLength, 0.2f, 5)), red, true);
        create_spring(prev_plank->get_node()->get_rigid_body(), plank->get_node()->get_rigid_body(), springLength, plankHalfLength, springStiffness, springDamping, 4.0f);
        create_spring(prev_plank->get_node()->get_rigid_body(), plank->get_node()->get_rigid_body(), springLength, plankHalfLength, springStiffness, springDamping, -4.0f);
        mat.translate(offsetBewteenPlanks, 0, 0);
        prev_plank = plank;
      }

      //Last plank
      create_spring(prev_plank->get_node()->get_rigid_body(), NULL, springLength, plankHalfLength, springStiffness, springDamping, 4.0f);
      create_spring(prev_plank->get_node()->get_rigid_body(), NULL, springLength, plankHalfLength, springStiffness, springDamping, -4.0f);

      // ground
      mat.loadIdentity();
      mat.translate(0, -1, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

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
  };
}

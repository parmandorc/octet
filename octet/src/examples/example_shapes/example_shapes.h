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

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 4, 0));

      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

	    // Left ball
      mat4t mat;
      mat.translate(-3, 15, 0);
      app_scene->add_shape(mat, new mesh_sphere(vec3(2, 2, 2), 2), red, true);

	    // Cube
      mat.loadIdentity();
      mat.translate(0, 10, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(2, 2, 2)), red, true);

	    // Right ball
      mat.loadIdentity();
      mat.translate( 3, 6, 0);
      app_scene->add_shape(mat, new mesh_cylinder(zcylinder(vec3(0, 0, 0), 2, 4)), blue, true);

      // swing
      mat.loadIdentity();
      mat.translate(0, 5, 0);
      mesh_instance *swing = app_scene->add_shape(mat, new mesh_box(vec3(4, 0.5f, 4)), green, true);
      btHingeConstraint *hinge = new btHingeConstraint(*swing->get_node()->get_rigid_body(), get_btVector3(vec3(0.0f, 0.0f, 0.0f)), get_btVector3(vec3(0.0f, 0.0f, 1.0f)));
      app_scene->add_constraint(hinge);
      
      // ball with spring
      mat.loadIdentity();
      mat.translate(-10, 15, 0);
      mesh_instance *base = app_scene->add_shape(mat, new mesh_box(vec3(1, 1, 1)), green, false);
      btTransform frameInBase = btTransform::getIdentity();
      frameInBase.setOrigin(get_btVector3(vec3(0.0f, -1.0f, 0.0f)));
      frameInBase.setRotation(btQuaternion(get_btVector3(vec3(1, 0, 0)), M__PI));
      mat.translate(0, -5, 0);
      mesh_instance *object = app_scene->add_shape(mat, new mesh_box(vec3(1, 1, 1)), blue, true);
      btTransform frameInObject = btTransform::getIdentity();
      frameInObject.setOrigin(get_btVector3(vec3(0.0f, 1.0f, 0.0f)));
      frameInObject.setRotation(btQuaternion(get_btVector3(vec3(1, 0, 0)), M__PI));
      btGeneric6DofSpringConstraint *spring = new btGeneric6DofSpringConstraint(*base->get_node()->get_rigid_body(), *object->get_node()->get_rigid_body(), frameInBase, frameInObject, true);
      spring->setLinearLowerLimit(get_btVector3(vec3(-5.0f, 0.0f, -5.0f)));
      spring->setLinearUpperLimit(get_btVector3(vec3(5.0f, 10.0f, 5.0f)));
      for (int i = 0; i < 6; i++) {
        spring->enableSpring(i, true);
        spring->setStiffness(i, (i < 3) ? 25.0f : 100.0f);
        spring->setDamping(i, (i < 3) ? 0.1f : 0.025f);
      }
      spring->setEquilibriumPoint(1, 5.0f);
      app_scene->add_constraint(spring);

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
        newSphere->get_node()->apply_central_force(vec3(randomizer.get(-200.0f, 50.0f), 0.0f, 0.0f));
	    }
    }
  };
}

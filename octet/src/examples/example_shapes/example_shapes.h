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

	int initialCount = 25;
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
		  counter = initialCount;

		  mat4t mat;
		  mat.loadIdentity();
		  mat.translate(0, 20, 0);
		  mesh_instance *newSphere = app_scene->add_shape(mat, new mesh_sphere(vec3(2, 2, 2), 1), new material(vec4(0.75f, 0.75f, 0.75f, 1)), true);
		  newSphere->get_node()->apply_central_force(vec3(randomizer.get(-250.0f, 250.0f), 0.0f, 0.0f));
	  }
    }
  };
}

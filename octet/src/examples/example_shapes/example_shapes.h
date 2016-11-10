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

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0.0f, 10, -20.0f));
      //app_scene->get_camera_instance(0)->get_node()->rotate(45, vec3(0, 1, 0));
      app_scene->get_camera_instance(0)->get_node()->rotate(-30, vec3(1, 0, 0));

      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

      mat4t mat;
      mat.loadIdentity();
      app_scene->add_shape(mat, new mesh_box(vec3(5.0f, 0.1f, 0.1f)), red, false);
      app_scene->add_shape(mat, new mesh_box(vec3(0.1f, 5.0f, 0.1f)), green, false);
      app_scene->add_shape(mat, new mesh_box(vec3(0.1f, 0.1f, 5.0f)), blue, false);

      mat.rotateY(30);
      mat.translate(1, 0, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(2.0f, 0.1f, 0.1f)), red, false);
      app_scene->add_shape(mat, new mesh_box(vec3(0.1f, 2.0f, 0.1f)), green, false);
      app_scene->add_shape(mat, new mesh_box(vec3(0.1f, 0.1f, 2.0f)), blue, false);
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
    }
  };
}

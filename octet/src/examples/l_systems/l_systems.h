////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  // Class for the sprite system.
  // Copied and modified from the example_invaderers project in octet.
  class sprite {
    // where is our sprite (overkill for a 2D game!)
    mat4t modelToWorld;

    // half the width of the sprite
    float halfWidth;

    // half the height of the sprite
    float halfHeight;

    // what texture is on our sprite
    int texture;

    // true if this sprite is enabled.
    bool enabled;

  public:
    sprite() {
      texture = 0;
      enabled = true;
    }

    sprite* init(int _texture, mat4t mat, float w, float h) {
      halfWidth = w * 0.5f;
      halfHeight = h * 0.5f;
      modelToWorld = mat;
      modelToWorld.translate(0, halfHeight, 0);
      texture = _texture;
      enabled = true;
      return this;
    }

    void render(texture_shader &shader, mat4t &cameraToWorld) {
      // invisible sprite
      if (!texture) return;

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      // use "old skool" rendering
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      shader.render(modelToProjection, 0);

      // this is an array of the positions of the corners of the sprite in 3D
      // a straight "float" here means this array is being generated here at runtime.
      float vertices[] = {
        -halfWidth, -halfHeight, 0,
        halfWidth, -halfHeight, 0,
        halfWidth,  halfHeight, 0,
        -halfWidth,  halfHeight, 0,
      };

      // attribute_pos (=0) is position of each corner
      // each corner has 3 floats (x, y, z)
      // there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
      glEnableVertexAttribArray(attribute_pos);

      // this is an array of the positions of the corners of the texture in 2D
      static const float uvs[] = {
        0,  0,
        1,  0,
        1,  1,
        0,  1,
      };

      // attribute_uv is position in the texture of each corner
      // each corner (vertex) has 2 floats (x, y)
      // there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
      glEnableVertexAttribArray(attribute_uv);

      // finally, draw the sprite (4 vertices)
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
    }

    bool &is_enabled() {
      return enabled;
    }
  };

  class l_system {
    // The axiom of the system
    std::string axiom;

    // The set of rules for generation
    std::map<char, std::string> rules;

    // The different iterations of the execution of the system
    // This are used to generate the sprites via turtle-graphics
    std::vector<std::string> iterations;

    // Executes the system to create the following iteration to the last one created.
    void runNextIteration() {
      std::string newIteration;
      
      for (std::string::iterator it = iterations.back().begin(); it != iterations.back().end(); ++it) {
        std::map<char, std::string>::iterator m_it = rules.find(*it);
        if (m_it != rules.end()) { //If a rule exists with this element...
          newIteration += (*m_it).second;
        }
        else { //If a rule doesn't exist, append character
          newIteration += *it;
        }
      }

      iterations.push_back(newIteration);
    }

  public:
    l_system() {}

    void init(std::string _axiom, std::map<char, std::string> _rules) {
      axiom = _axiom;
      rules = _rules;
      iterations.clear();
      iterations.push_back(axiom);
    }

    std::string getIteration(unsigned int i) {
      while (iterations.size() <= i) {
        runNextIteration();
      }

      return iterations[i];
    }

  };

  /// Scene containing the l-systems app
  class l_systems : public app {

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a textured triangle
    texture_shader texture_shader_;

    // a texture for our text
    GLuint font_texture;

    // information for our text
    bitmap_font font;

    // collection of sprites
    std::vector<sprite*> sprites;

    // generative l-system to use
    l_system lsystem;

    void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      modelToWorld.scale(scale, scale, 1);
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      /*mat4t tmp;
      glLoadIdentity();
      glTranslatef(x, y, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
      glScalef(scale, scale, 1);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

      enum { max_quads = 32 };
      bitmap_font::vertex vertices[max_quads * 4];
      uint32_t indices[max_quads * 6];
      aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

      unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, font_texture);

      shader.render(modelToProjection, 0);

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x);
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u);
      glEnableVertexAttribArray(attribute_uv);

      glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    // this generates a set of sprites from the given string and parameters
    std::vector<sprite*> turtleGraphics(std::string str, float angle) {
      GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
      std::vector<sprite*> sprites;
      std::vector<mat4t> matstack;
      mat4t mat;
      mat.loadIdentity();

      for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (*it == '[') {
          matstack.push_back(mat);
          mat.rotateZ(angle);
        }
        else if (*it == ']') {
          mat = matstack.back();
          matstack.pop_back();
          mat.rotateZ(-angle);
        }
        else {
          sprites.push_back((new sprite())->init(white, mat, 0.2f, 1));
          mat.translate(0, 1, 0);
        }
      }
      return sprites;
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      texture_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 6, 10);

      font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

      // Create the l-system class object
      std::string axiom = "0";
      std::map<char, std::string> rules;
      rules['1'] = "11";
      rules['0'] = "1[0]0";
      lsystem.init(axiom, rules);

      sprites = turtleGraphics(lsystem.getIteration(4), 30);
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw all the sprites
      std::for_each(sprites.begin(), sprites.end(), [this](sprite* s) { s->render(texture_shader_, cameraToWorld); });

      char text[32];
      sprintf(text, "L-systems\n");
      //draw_text(texture_shader_, -1.75f, 2, 1.0f / 256, text);
    }
  };
}

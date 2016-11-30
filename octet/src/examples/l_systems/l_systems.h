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

    void render(texture_shader &shader, mat4t &cameraToWorld, float width = -1) {
      // invisible sprite
      if (!texture) return;

      float halfWidth = width < 0 ? this->halfWidth : width * 0.5f;

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
    void setTransform(mat4t mat) {
      modelToWorld = mat;
    }

    bool &is_enabled() {
      return enabled;
    }

    mat4t getModelToWorld() {
      return modelToWorld;
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

  struct config {
    char name[32];
    unsigned int n;
    float angle;
    std::string axiom;
    std::map<char, std::string> rules;
    std::string ignored;
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

    // sprite used for UI overlay
    sprite UI_BackgroundBox;

    // whether to show or hide the UI
    bool showUI;

    // generative l-system to use
    l_system lsystem;

    // parameters of the l-system
    struct config conf;

    void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
      mat4t modelToWorld = cameraToWorld;
      modelToWorld.translate(x, y, -1);
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

    bool loadConfigFile(struct config &conf, const char* file) {
      std::ifstream is(file);
      if (!is.good()) {
        is.close(); return false;
      }

      sprintf(conf.name, "%s", file);

      // store the line here
      char buffer[256];

      // loop over lines
      while (!is.eof()) {
        is.getline(buffer, sizeof(buffer));

        // loop over columns
        char *b = buffer;

        if (*b == '#') continue; //skip commented line

        // Read parameter
        while (*b == ' ' || *b == '\t') ++b; //trim white space
        char *e = b;
        while (*e != 0 && *e != '=' && *e != ':' && *e != ' ' && *e != '\t') ++e; //get word
        std::string param = std::string(b, e);
        b = e + 1;
        while (*b == ' ' || *b == '\t' || *b == '=' || *b == ':') ++b; //trim white space until value
        e = b;
        while (*e != 0 && *e != ' ' && *e != '\t') ++e; //trim white space

        if (!param.compare("n")) {
          conf.n = std::atoi(b);
        }
        else if (!param.compare("angle")) {
          conf.angle = std::atof(b);
        }
        else if (!param.compare("axiom")) {
          conf.axiom = std::string(b, e);
        }
        else if (!param.compare("rule")) {
          if (!is.eof()) {
            char c = *b;
            std::string rule;

            is.getline(buffer, sizeof(buffer));
            b = buffer;
            while (*b != 0) {
              if (*b != ' ' && *b != '\t') {
                rule += *b;
              }
              ++b;
            }
            conf.rules[c] = rule;
          }
        }
        else if (!param.compare("ignored")) {
          while (*b != 0) {
            if (*b != ' ' && *b != '\t' && *b != ',') {
              conf.ignored += *b;
            }
            ++b;
          }
        }
      }

      is.close();
      return true;
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    // this generates a set of sprites from the given string and parameters
    std::vector<sprite*> turtleGraphics(std::string str, float angle, std::string ignored = "") {
      GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
      std::vector<sprite*> sprites;
      std::vector<mat4t> matstack;
      mat4t mat;
      mat.loadIdentity();

      for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (*it == '[') {
          matstack.push_back(mat);
        }
        else if (*it == ']') {
          mat = matstack.back();
          matstack.pop_back();
        }
        else if (*it == '+') {
          mat.rotateZ(angle);
        }
        else if (*it == '-') {
          mat.rotateZ(-angle);
        }
        else if (ignored.find_first_of(*it) == std::string::npos) {
          sprites.push_back((new sprite())->init(white, mat, 0, 1));
          mat.translate(0, 1, 0);
        }
      }
      return sprites;
    }

    mat4t centreCameraOnSprites(std::vector<sprite*> sprites) {
      vec4 min, max;
      min = max = (*sprites.begin())->getModelToWorld().w();
      for (std::vector<sprite*>::iterator it = sprites.begin(); it != sprites.end(); ++it) {
        min = (*it)->getModelToWorld().w().min(min);
        max = (*it)->getModelToWorld().w().max(max);
      }

      mat4t mat;
      mat.loadIdentity();
      mat.w() = (min + max) * 0.5f;
      vec4 d = max - min;
      mat.translate(0, 0, (d[0] > d[1] ? d[0] : d[1]) * 0.75f + 1.0f);
      return mat;
    }

    void input() {

      // Cycle through config files
      for (int i = 1; i <= 8; ++i) {
        if (is_key_going_down('0' + i)) {
          char filePath[32];
          sprintf(filePath, "config/config%d.txt", i);
          if (loadConfigFile(conf, filePath)) {
            lsystem.init(conf.axiom, conf.rules);
            std::for_each(sprites.begin(), sprites.end(), [](sprite* s) { free(s); });
            sprites = turtleGraphics(lsystem.getIteration(conf.n), conf.angle, conf.ignored);
            cameraToWorld = centreCameraOnSprites(sprites);
          }
        }
      }

      // Cycle through iterations
      if (is_key_going_down(key_right) || is_key_going_down(key_left)) {
        conf.n += is_key_going_down(key_right) ? 1 : (conf.n > 1 ? -1 : 0);
        std::for_each(sprites.begin(), sprites.end(), [](sprite* s) { free(s); });
        sprites = turtleGraphics(lsystem.getIteration(conf.n), conf.angle, conf.ignored);
        cameraToWorld = centreCameraOnSprites(sprites);
      }

      // Toggle UI
      if (is_key_going_down(key_esc))
        showUI = !showUI;
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      texture_shader_.init();

      // load texture for text
      font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

      // Load the l-system from the default config file
      if (loadConfigFile(conf, "config/config1.txt")) {
        lsystem.init(conf.axiom, conf.rules);
        sprites = turtleGraphics(lsystem.getIteration(conf.n), conf.angle, conf.ignored);
        cameraToWorld = centreCameraOnSprites(sprites);
      }

      // Create the UI permanent elements
      GLuint bgColor = resource_dict::get_texture_handle(GL_RGBA, "#00000077");
      UI_BackgroundBox.init(bgColor, mat4t().loadIdentity(), 0.75f, 0.3f);
      showUI = false;
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      input();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0.5f, 0.5f, 0.5f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw all the sprites
      std::for_each(sprites.begin(), sprites.end(), [this](sprite* s) { s->render(texture_shader_, cameraToWorld, cameraToWorld.w()[2] * 0.00266667f); });

      // draw UI
      if (showUI) {
        //background panel
        mat4t mat = cameraToWorld;
        mat.translate(-0.6f, 0.8125f, -1);
        UI_BackgroundBox.setTransform(mat);
        UI_BackgroundBox.render(texture_shader_, cameraToWorld);

        //file name
        draw_text(texture_shader_, -0.75f, 0.75f, 0.00075f, conf.name);

        //iteration number
        char text[32];
        sprintf(text, "Iteration (n): %d", conf.n);
        draw_text(texture_shader_, -0.75f, 0.65f, 0.00075f, text);

        //angle value
        sprintf(text, "Angle: %.1f", conf.angle);
        draw_text(texture_shader_, -0.75f, 0.6f, 0.00075f, text);
      }
    }
  };
}

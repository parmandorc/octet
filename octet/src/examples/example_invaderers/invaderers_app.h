////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// invaderer example: simple game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//

namespace octet {
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

    //True if this sprite has active functionality. (The sprite migth be enabled so it moves, but might not be able to interact with other elements)
    bool active;

    // true if this sprite should be rendered in false 3D
    bool is3D;

    // true if this sprite should be tinted with specified color
    bool doApplyTint;

    // the color (without alpha channel) that the sprite should be tinted with
    vec3 colorTint;

    //Tracks the position of the sprite
    vec2 position;

  public:
    sprite() {
      texture = 0;
      enabled = true;
    }

    void init(int _texture, float x, float y, float w, float h, bool _is3D = false, bool _doApplyTint = false, vec3 _colorTint = vec3(0.0f, 0.0f, 0.0f)) {
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      position = vec2(x, y);
      halfWidth = w * 0.5f;
      halfHeight = h * 0.5f;
      texture = _texture;
      is3D = _is3D;
      doApplyTint = _doApplyTint;
      colorTint = _colorTint;
      enabled = true;
      active = true;
    }

    void render(texture_shader &shader, mat4t &cameraToWorld) {
      // invisible sprite... used for gameplay.
      if (!texture) return;

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      // Set render uniform parameters
      GLuint program = shader.get_program();
      GLint is3DLoc = glGetUniformLocation(program, "is3D");
      if (is3DLoc != -1) glUniform1i(is3DLoc, is3D);
      GLint doApplyTintLoc = glGetUniformLocation(program, "doApplyTint");
      if (doApplyTintLoc != -1) glUniform1i(doApplyTintLoc, doApplyTint);
      GLint colorTintLoc = glGetUniformLocation(program, "colorTint");
      if (colorTintLoc != -1) glUniform3f(colorTintLoc, colorTint[0], colorTint[1], colorTint[2]);

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
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
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
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)uvs );
      glEnableVertexAttribArray(attribute_uv);
    
      // finally, draw the sprite (4 vertices)
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
      position += vec2(x, y);
    }

    vec2 get_position() {
      return position;
    }

    //move the object to the specified position
    void set_position(float x, float y) {
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      position = vec2(x, y);
    }

    // position the object relative to another.
    void set_relative(sprite &rhs, float x, float y) {
      modelToWorld = rhs.modelToWorld;
      modelToWorld.translate(x, y, 0);
      position = rhs.position + vec2(x, y);
    }

    void get_size(float *w, float *h) {
      *w = halfWidth * 2.0f;
      *h = halfHeight * 2.0f;
    }

    // changes the size of the sprite
    void set_size(float w, float h) {
      halfWidth = w * 0.5f;
      halfHeight = h * 0.5f;
    }

    // changes the color tint of the sprite
    void set_color_tint(bool _doApplyTint, vec3 _colorTint = vec3(0, 0, 0)) {
      doApplyTint = _doApplyTint;
      colorTint = _colorTint;
    }

    // return true if this sprite collides with another.
    // note the "const"s which say we do not modify either sprite
    bool collides_with(const sprite &rhs) const {
      float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
      float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

      // both distances have to be under the sum of the halfwidths
      // for a collision
      return
        (fabsf(dx) < halfWidth + rhs.halfWidth) &&
        (fabsf(dy) < halfHeight + rhs.halfHeight)
      ;
    }

    // return true if the sprite collides with the specified circle
    // note: approximation by considering if one of the corners is inside the circle
    bool collides_with(const float x, const float y, const float r) {
      float c00_x = modelToWorld[3][0] + halfWidth;
      float c00_y = modelToWorld[3][1] + halfHeight;
      float c01_x = modelToWorld[3][0] + halfWidth;
      float c01_y = modelToWorld[3][1] - halfHeight;
      float c10_x = modelToWorld[3][0] - halfWidth;
      float c10_y = modelToWorld[3][1] + halfHeight;
      float c11_x = modelToWorld[3][0] - halfWidth;
      float c11_y = modelToWorld[3][1] - halfHeight;

      return
        ((x - c00_x) * (x - c00_x) + (y - c00_y) * (y - c00_y) < r * r) ||
        ((x - c01_x) * (x - c01_x) + (y - c01_y) * (y - c01_y) < r * r) ||
        ((x - c10_x) * (x - c10_x) + (y - c10_y) * (y - c10_y) < r * r) ||
        ((x - c11_x) * (x - c11_x) + (y - c11_y) * (y - c11_y) < r * r)
      ;
    }

    bool is_above(const sprite &rhs, float margin) const {
      float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

      return
        (fabsf(dx) < halfWidth + margin)
      ;
    }

    bool &is_enabled() {
      return enabled;
    }

    bool &is_active() {
      return active;
    }
  };

  class invaderers_app : public octet::app {
    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a textured triangle
    texture_shader texture_shader_;

    enum {
      num_sound_sources = 8,
      num_missiles = 2,
      num_nukes = 2,
      num_bombs = 2,
      num_borders = 4,
      num_invaderers = 100,
      num_heals = 5,
      num_powerups = 5,
      num_nuke_pickups = 5,
      num_background_sprites = 50,

      // sprite definitions
      ship_sprite = 0,

      // background
      first_background_sprite,
      last_background_sprite = first_background_sprite + num_background_sprites - 1,

      // entities
      boss_sprite,
      first_invaderer_sprite,
      last_invaderer_sprite = first_invaderer_sprite + num_invaderers - 1,

      // pickups
      first_heal_sprite,
      last_heal_sprite = first_heal_sprite + num_heals - 1,

      first_powerup_sprite,
      last_powerup_sprite = first_powerup_sprite + num_powerups - 1,

      first_nuke_pickup_sprite,
      last_nuke_pickup_sprite = first_nuke_pickup_sprite + num_nuke_pickups - 1,

      // bullets
      first_missile_sprite,
      last_missile_sprite = first_missile_sprite + num_missiles - 1,

      first_bomb_sprite,
      last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

      first_nuke_sprite,
      last_nuke_sprite = first_nuke_sprite + num_nukes - 1,

      // borders
      first_border_sprite,
      last_border_sprite = first_border_sprite + num_borders - 1,

      //GUI
      game_over_sprite,
      win_sprite,
      title_sprite,
      normal_button_sprite,
      hardcore_button_sprite,
      nukes_available_sprite,
      powerup_available_sprite,

      num_sprites,

    };

    // timers for missiles and bombs
    int missiles_disabled;
    int nukes_disabled;
    int bombs_disabled;

    // accounting for bad guys
    int num_lives;

    // game state
    enum {
      MAIN_MENU,
      NORMAL,
      HARDCORE
    } gamemode;
    bool game_over;
    bool game_win;
    unsigned int score;
    bool hardcore_unlocked;
    unsigned int normal_highscore;
    unsigned int hardcore_highscore;

    // speed of the elements in the scene
    float scene_velocity;

    // sounds
    ALuint whoosh;
    ALuint bang;
    ALuint cling;
    ALuint tap;
    unsigned cur_source;
    ALuint sources[num_sound_sources];

    // big array of sprites
    sprite sprites[num_sprites];

    //Information for handling new rows of enemies
    unsigned int frames_between_rows;
    int counter_for_next_row;
    std::vector<unsigned int> available_invaderers;
    enum RowElement{
      None,
      Invaderer,
      Boss,
      Heal,
      Powerup,
      Nuke,
    };
    std::vector<std::vector<enum RowElement>> rows;
    int current_row;
    int boss_lives;
    int boss_disabled;

    // pickups information
    int nukes_available;
    bool powerup_available;
    int powerup_left;

    // random number generator
    class random randomizer;

    // a texture for our text
    GLuint font_texture;

    // information for our text
    bitmap_font font;

    ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

    //returns true if the player won
    bool is_game_win() {
      return gamemode == NORMAL && current_row >= rows.size() && available_invaderers.size() == num_invaderers && boss_lives <= 0;
    }

    // called when win the normal game mode
    void on_game_win() {
      game_win = true;
      sprites[win_sprite].translate(-20, 0);
      hardcore_unlocked = true;
      sprites[hardcore_button_sprite].set_color_tint(false);
      if (score > normal_highscore)
        normal_highscore = score;
    }

    // called on game over
    void on_game_over() {
      game_over = true;
      sprites[game_over_sprite].translate(-20, 0);

      unsigned int *highscore = (gamemode == NORMAL ? &normal_highscore : &hardcore_highscore);
      if (score > *highscore)
        *highscore = score;
    }

    // called when we hit an enemy
    void on_hit_invaderer() {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);

      //add up score
      ++score;

      //check for game win
      if (is_game_win())
        on_game_win();
    }

    // called when we are hit
    void on_hit_ship() {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);

      if (--num_lives == 0 && !game_over && !game_win)
        on_game_over();

      if (num_lives < 0) {
        num_lives = 0;
      }
    }

    //Called when an invaderer passes the ship
    void on_invaderer_pass() {
      if (--num_lives == 0 && !game_over && !game_win)
        on_game_over();

      if (num_lives < 0) {
        num_lives = 0;
      }
    }

    //Called when boss hits or passes ship
    void on_boss_win() {
      num_lives = 0;

      if (!game_over && !game_win)
        on_game_over();
    }

    //Called when hit boss
    void on_hit_boss(int damage = 1) {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);

      if (boss_lives > 0) {
        boss_lives -= damage;
        if (boss_lives <= 0) {
          boss_lives = 0;

          score += 10;
        }
      }

      if (is_game_win())
        on_game_win();
    }

    // process the detonation of a nuke
    //When collides with an enemy, explodes an kills all enemies in the area
    void on_nuke_detonation(float x, float y) {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);

      for (int i = 0; i != num_invaderers; ++i) { //process affected invaderers
        sprite &invaderer = sprites[first_invaderer_sprite + i];
        if (invaderer.is_enabled() && invaderer.collides_with(x, y, 1.25f)) {
          invaderer.is_enabled() = false;
          invaderer.translate(20, 0);
          available_invaderers.push_back(i);

          on_hit_invaderer();
        }
      }

      sprite &boss = sprites[boss_sprite]; //check if hit boss
      if (boss.is_enabled() && boss.collides_with(x, y, 1.25f)) {
        on_hit_boss(5);

        if (boss_lives <= 0) {
          boss.is_enabled() = false;
          boss.translate(20, 0);
        }
        else {
          boss.set_size(0.5f + 1.5f * (boss_lives - 1) / 9, 0.5f + 1.5f * (boss_lives - 1) / 9);
        }
      }
    }

    // use the keyboard to move the ship
    void move_ship() {
      const float ship_speed = powerup_left > 0 ? 0.1f : 0.05f;

      // left and right arrows
      if (is_key_down(key_left)) {
        sprites[ship_sprite].translate(-ship_speed, 0);
        if (sprites[ship_sprite].collides_with(sprites[first_border_sprite+2])) {
          sprites[ship_sprite].translate(+ship_speed, 0);
        }
      } else if (is_key_down(key_right)) {
        sprites[ship_sprite].translate(+ship_speed, 0);
        if (sprites[ship_sprite].collides_with(sprites[first_border_sprite+3])) {
          sprites[ship_sprite].translate(-ship_speed, 0);
        }
      }
    }

    void update_powerup() {
      // launch powerup
      if (powerup_available && powerup_left <= 0 && is_key_going_down('Z')) {
        ALuint source = get_sound_source();
        alSourcei(source, AL_BUFFER, cling);
        alSourcePlay(source);
        sprites[powerup_available_sprite].translate(20, 0);

        powerup_available = false;
        powerup_left = 75;
        sprites[ship_sprite].translate(0, 0.125f);
        sprites[ship_sprite].set_size(0.5f, 0.5f);
        sprites[ship_sprite].set_color_tint(true, vec3(1.0f, 1.0f, 0.0f));
      }

      if (powerup_left > 0) {
        --powerup_left;

        //check if powerup ends
        if (powerup_left == 0) {
          sprites[ship_sprite].translate(0, -0.125f);
          sprites[ship_sprite].set_size(0.25f, 0.25f);
          sprites[ship_sprite].set_color_tint(false);
        }
      }
    }

    // fire button (space)
    void fire_missiles() {
      if (missiles_disabled) {
        --missiles_disabled;
      } else if (is_key_going_down(' ')) {
        // find a missile
        for (int i = 0; i != num_missiles; ++i) {
          if (!sprites[first_missile_sprite+i].is_enabled()) {
            sprites[first_missile_sprite+i].set_relative(sprites[ship_sprite], 0, 0.5f);
            sprites[first_missile_sprite+i].is_enabled() = true;
            missiles_disabled = 5;
            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, tap);
            alSourcePlay(source);
            break;
          }
        }
      }
    }

    // fire button (x)
    void fire_nukes() {
      if (nukes_disabled) {
        --nukes_disabled;
      }
      else if (nukes_available > 0 && is_key_going_down('X')) {
        // find a nuke
        for (int i = 0; i != num_nukes; ++i) {
          if (!sprites[first_nuke_sprite + i].is_enabled()) {
            sprites[first_nuke_sprite + i].set_relative(sprites[ship_sprite], 0, 0.5f);
            sprites[first_nuke_sprite + i].is_enabled() = true;
            nukes_disabled= 5;
            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, whoosh);
            alSourcePlay(source);

            --nukes_available;
            break;
          }
        }
      }
    }

    // pick and invader and fire a bomb
    void fire_bombs() {
      if (bombs_disabled) {
        --bombs_disabled;
      } else {
        // find an invaderer
        sprite &ship = sprites[ship_sprite];
        for (int j = randomizer.get(0, num_invaderers); j < num_invaderers; ++j) {
          sprite &invaderer = sprites[first_invaderer_sprite+j];
          if (invaderer.is_enabled() && invaderer.is_above(ship, 0.3f)) {
            // find a bomb
            for (int i = 0; i != num_bombs; ++i) {
              if (!sprites[first_bomb_sprite+i].is_enabled()) {
                sprites[first_bomb_sprite+i].set_relative(invaderer, 0, -0.25f);
                sprites[first_bomb_sprite+i].is_enabled() = true;
                bombs_disabled = 30;
                ALuint source = get_sound_source();
                alSourcei(source, AL_BUFFER, whoosh);
                alSourcePlay(source);
                return;
              }
            }
            return;
          }
        }
      }
    }

    // animate the missiles
    void move_missiles() {
      const float missile_speed = 0.3f;
      for (int i = 0; i != num_missiles; ++i) {
        sprite &missile = sprites[first_missile_sprite+i];
        if (missile.is_enabled()) {
          missile.translate(0, missile_speed);
          for (int j = 0; j != num_invaderers; ++j) {
            sprite &invaderer = sprites[first_invaderer_sprite+j];
            if (invaderer.is_enabled() && missile.collides_with(invaderer)) {
              missile.is_enabled() = false;
              missile.translate(20, 0);
              
              invaderer.is_enabled() = false;
              invaderer.translate(20, 0);
              available_invaderers.push_back(j);

              on_hit_invaderer();

              goto next_missile;
            }
          }

          //Check is missile hits boss
          sprite &boss = sprites[boss_sprite];
          if (boss.is_enabled() && missile.collides_with(boss)) {
            missile.is_enabled() = false;
            missile.translate(20, 0);

            on_hit_boss();

            if (boss_lives <= 0) {
              boss.is_enabled() = false;
              boss.translate(20, 0);
            }
            else {
              boss.set_size(0.5f + 1.5f * (boss_lives - 1) / 9, 0.5f + 1.5f * (boss_lives - 1) / 9);
            }

            goto next_missile;
          }

          if (missile.collides_with(sprites[first_border_sprite+1])) {
            missile.is_enabled() = false;
            missile.translate(20, 0);
          }
        }
      next_missile:;
      }
    }

    // animate the nukes
    void move_nukes() {
      const float nuke_speed = 0.3f;
      for (int i = 0; i != num_nukes; ++i) {
        sprite &nuke = sprites[first_nuke_sprite + i];
        if (nuke.is_enabled()) {
          nuke.translate(0, nuke_speed);

          //check if nuke hits an invaderer
          for (int j = 0; j != num_invaderers; ++j) {
            sprite &invaderer = sprites[first_invaderer_sprite + j];
            if (invaderer.is_enabled() && nuke.collides_with(invaderer)) {
              vec2 centre = nuke.get_position();
              nuke.is_enabled() = false;
              nuke.translate(20, 0);
              
              on_nuke_detonation(centre[0], centre[1]);

              goto next_nuke;
            }
          }

          //check if nuke hits boss
          sprite &boss = sprites[boss_sprite];
          if (boss.is_enabled() && nuke.collides_with(boss)) {
            vec2 centre = nuke.get_position();
            nuke.is_enabled() = false;
            nuke.translate(20, 0);

            on_nuke_detonation(centre[0], centre[1]);

            goto next_nuke;
          }

          //check if nuke hits border
          if (nuke.collides_with(sprites[first_border_sprite + 1])) {
            nuke.is_enabled() = false;
            nuke.translate(20, 0);
          }
        }
      next_nuke:;
      }
    }

    // animate the bombs
    void move_bombs() {
      const float bomb_speed = 0.2f;
      for (int i = 0; i != num_bombs; ++i) {
        sprite &bomb = sprites[first_bomb_sprite+i];
        if (bomb.is_enabled()) {
          bomb.translate(0, -bomb_speed);
          if (bomb.collides_with(sprites[ship_sprite])) {
            bomb.is_enabled() = false;
            bomb.translate(20, 0);
            bombs_disabled = 50;
            if (powerup_left <= 0)
              on_hit_ship();
            goto next_bomb;
          }
          if (bomb.collides_with(sprites[first_border_sprite+0])) {
            bomb.is_enabled() = false;
            bomb.translate(20, 0);
          }
        }
      next_bomb:;
      }
    }

    // move the array of enemies
    void move_invaders(float dx, float dy) {
      for (int j = 0; j != num_invaderers; ++j) {
        sprite &invaderer = sprites[first_invaderer_sprite + j];
        if (invaderer.is_enabled()) {
          invaderer.translate(dx, dy);

          if (invaderer.collides_with(sprites[ship_sprite])) { //Check if the invaderer hit the ship
            invaderer.is_enabled() = false;
            invaderer.translate(20, 0);
            available_invaderers.push_back(j);

            on_hit_invaderer();
            if (powerup_left <= 0)
              on_hit_ship();
          }

          if (invaderer.is_active() && invaderer.collides_with(sprites[first_border_sprite + 0])) { //Check if the invaderer passed the ship
            invaderer.is_active() = false;
            on_invaderer_pass();
          }

          if (invaderer.get_position()[1] < -3.125f) { //Check if the invaderer is out of screen
            invaderer.is_enabled() = false;
            invaderer.translate(20, 0);
            available_invaderers.push_back(j);

            if (is_game_win()) //Check if game ended with this last invaderer passing
              on_game_win();
          }
        }
      }
    }

    //move boss
    void move_boss(float dx, float dy) {
      sprite &boss = sprites[boss_sprite];
      if (boss.is_enabled()) {
        boss.translate(dx, dy);

        if (boss.collides_with(sprites[ship_sprite])) {
          on_hit_ship();
          on_boss_win();
        }
        else if (boss.collides_with(sprites[first_border_sprite + 0])) {
          on_boss_win();
        }
      }
    }

    // moves health packs
    void move_health_packs(float dx, float dy) {
      for (int i = 0; i != num_heals; ++i) {
        sprite &heal = sprites[first_heal_sprite + i];
        if (heal.is_enabled()) {
          heal.translate(dx, dy);

          if (heal.collides_with(sprites[ship_sprite])) { //Check if the ship picked up the health pack
            heal.is_enabled() = false;
            heal.translate(20, 0);

            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, cling);
            alSourcePlay(source);

            ++num_lives;
          }
          else if (heal.get_position()[1] < -3.125f) { //Check if the sprite is out of screen
            heal.is_enabled() = false;
            heal.translate(20, 0);
          }
        }
      }
    }

    // moves powerups
    void move_powerups(float dx, float dy) {
      for (int i = 0; i != num_powerups; ++i) {
        sprite &powerup = sprites[first_powerup_sprite + i];
        if (powerup.is_enabled()) {
          powerup.translate(dx, dy);

          if (powerup.collides_with(sprites[ship_sprite])) { //Check if the ship picked up the powerup
            powerup.is_enabled() = false;
            powerup.translate(20, 0);

            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, cling); //TO-DO: change sound
            alSourcePlay(source);

            if (!powerup_available)
              sprites[powerup_available_sprite].translate(-20, 0);

            powerup_available = true;
          }
          else if (powerup.get_position()[1] < -3.125f) { //Check if the sprite is out of screen
            powerup.is_enabled() = false;
            powerup.translate(20, 0);
          }
        }
      }
    }

    // moves nuke pickups
    void move_nuke_pickups(float dx, float dy) {
      for (int i = 0; i != num_nuke_pickups; ++i) {
        sprite &nuke_pickup = sprites[first_nuke_pickup_sprite + i];
        if (nuke_pickup.is_enabled()) {
          nuke_pickup.translate(dx, dy);

          if (nuke_pickup.collides_with(sprites[ship_sprite])) { //Check if the ship picked up the nuke
            nuke_pickup.is_enabled() = false;
            nuke_pickup.translate(20, 0);

            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, cling); //TO-DO: change sound
            alSourcePlay(source);

            ++nukes_available;
          }
          else if (nuke_pickup.get_position()[1] < -3.125f) { //Check if the sprite is out of screen
            nuke_pickup.is_enabled() = false;
            nuke_pickup.translate(20, 0);
          }
        }
      }
    }

    // moves the different pickups in the scene
    void move_pickups(float dx, float dy) {

      move_health_packs(dx, dy);

      move_powerups(dx, dy);

      move_nuke_pickups(dx, dy);
    }

    // moves the background
    void move_background(float dx, float dy) {
      for (int i = 0; i != num_background_sprites; ++i) {
        sprite &background_sprite = sprites[first_background_sprite + i];
        if (background_sprite.is_enabled()) {
          float w, h;
          background_sprite.get_size(&w, &h);
          background_sprite.translate(dx, dy * h);

          if (background_sprite.get_position()[1] < -3.125f) { //Check if the sprite is out of screen
            background_sprite.translate(0, 6.25f);
          }
        }
      }
    }

    // builds a row for hardcore mode
    std::vector<enum RowElement> build_hardcore_row() {
      std::vector<enum RowElement> row;

      if (boss_disabled > 0)
        --boss_disabled;
      if (boss_disabled == 0 && !sprites[boss_sprite].is_enabled() 
        && randomizer.get(0.0f, 1.0f) < 0.01f + current_row * 0.00025f) { // boss - 1% in every row, with increasing probability
        row.push_back(RowElement::Boss);
        boss_disabled = 5;
      }
      else {
        int num_cols = randomizer.get(9, 12); // give variance to columns layout
        for (int i = 0; i < num_cols; ++i) {
          if ((boss_disabled == 0 || fabsf(i - 0.5f * (num_cols - 1)) > 2) && // avoid putting elements over the boss
            randomizer.get(0.0f, 1.0f) < 0.05f + current_row * 0.0015f) { // rows have increasingly more elements
            float dice = randomizer.get(0.0f, 1.0f);
            if (dice < 0.025f)
              row.push_back(RowElement::Heal); // heal - 2.5% in each position
            else if (dice < 0.05f)
              row.push_back(RowElement::Nuke); // nuke - 2.5% in each position
            else if (dice < 0.075f)
              row.push_back(RowElement::Powerup); // powerup - 2.5% each position
            else
              row.push_back(RowElement::Invaderer); // invaderer - 92.5% in each position
          }
          else {
            row.push_back(RowElement::None);
          }
        }
      }

      return row;
    }

    //Spawn a new row in the scene
    void spawn_new_row() {
      std::vector<enum RowElement> row;
      if (gamemode == NORMAL && current_row < rows.size()) {
        //Get the next row
        row = rows[current_row];
        ++current_row;
      }
      else if (gamemode == HARDCORE) {
        row = build_hardcore_row();
        ++current_row;
      }

      int num_cols = row.size();
      float step = 5.75f / (num_cols + 1); //factor out so the division is computed only once
      for (int i = 0; i != num_cols; ++i) {
        switch (row[i]) {

        case RowElement::Invaderer: //Spawn invaderer in that position
        {
          if (!available_invaderers.empty()) {
            //Pop the index for the first available invaderer
            unsigned int sprite_index = available_invaderers[0];
            available_invaderers.erase(available_invaderers.begin());

            sprite &invaderer = sprites[first_invaderer_sprite + sprite_index];
            invaderer.set_position(-2.875f + step * (float)(i + 1), 3.125f);
            invaderer.is_enabled() = true;
            invaderer.is_active() = true;
          }
        }
        break;

        case RowElement::Heal:
        {
          for (int j = 0; j < num_heals; ++j) { //Spawn health pack
            sprite &heal = sprites[first_heal_sprite + j];
            if (!heal.is_enabled()) {
              heal.set_position(-2.875f + step * (float)(i + 1), 3.125f);
              heal.is_enabled() = true;
              break;
            }
          }
        }
        break;

        case RowElement::Powerup:
        {
          for (int j = 0; j < num_powerups; ++j) { //Spawn health pack
            sprite &powerup = sprites[first_powerup_sprite + j];
            if (!powerup.is_enabled()) {
              powerup.set_position(-2.875f + step * (float)(i + 1), 3.125f);
              powerup.is_enabled() = true;
              break;
            }
          }
        }
        break;

        case RowElement::Nuke:
        {
          for (int j = 0; j < num_nuke_pickups; ++j) { //Spawn health pack
            sprite &nuke_pickup = sprites[first_nuke_pickup_sprite + j];
            if (!nuke_pickup.is_enabled()) {
              nuke_pickup.set_position(-2.875f + step * (float)(i + 1), 3.125f);
              nuke_pickup.is_enabled() = true;
              break;
            }
          }
        }
        break;

        case RowElement::Boss:
        {
          if (!sprites[boss_sprite].is_enabled()) { //Spawn boss if available (only one boss at a time in the world)
            sprite &boss = sprites[boss_sprite];
            boss.set_position(-2.875f + step * (float)(i + 1), 3.875f);
            boss.set_size(2.0f, 2.0f);
            boss_lives = 10;
            boss.is_enabled() = true;
          }
        }
        break;

        }
      }
    }

    // check if any invaders hit the sides.
    bool invaders_collide(sprite &border) {
      for (int j = 0; j != num_invaderers; ++j) {
        sprite &invaderer = sprites[first_invaderer_sprite+j];
        if (invaderer.is_enabled() && invaderer.collides_with(border)) {
          return true;
        }
      }
      return false;
    }

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

      enum { max_quads = 64 };
      bitmap_font::vertex vertices[max_quads*4];
      uint32_t indices[max_quads*6];
      aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

      unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, font_texture);

      GLuint program = shader.get_program();
      GLint is3DLoc = glGetUniformLocation(program, "is3D");
      if (is3DLoc != -1) glUniform1i(is3DLoc, false);
      GLint doApplyTintLoc = glGetUniformLocation(program, "doApplyTint");
      if (doApplyTintLoc != -1) glUniform1i(doApplyTintLoc, false);

      shader.render(modelToProjection, 0);

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x );
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u );
      glEnableVertexAttribArray(attribute_uv);

      glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
    }

  public:

    // this is called when we construct the class
    invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      texture_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 3);

      font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

      GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
      sprites[ship_sprite].init(ship, 0, -2.75f, 0.25f, 0.25f, true);

      GLuint invaderer = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invaderer.gif");
      for (int i = 0; i != num_invaderers; ++i) {
        assert(first_invaderer_sprite + i <= last_invaderer_sprite);
        sprites[first_invaderer_sprite + i].init(
          invaderer, 20, 0, 0.5f, 0.5f, true, true, vec3(1.0f, 0.0f, 0.0f)
        );
        sprites[first_invaderer_sprite + i].is_enabled() = false;
        available_invaderers.push_back(i);
      }
      sprites[boss_sprite].init(invaderer, 20, 0, 2.0f, 2.0f, true, true, vec3(1.0f, 0.0f, 0.0f));
      sprites[boss_sprite].is_enabled() = false;

      // set the border to white for clarity
      GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
      sprites[first_border_sprite + 0].init(white, 0, -3, 6, 0.2f);
      sprites[first_border_sprite + 1].init(white, 0, 3, 6, 0.2f);
      sprites[first_border_sprite + 2].init(white, -3, 0, 0.2f, 6);
      sprites[first_border_sprite + 3].init(white, 3, 0, 0.2f, 6);

      // use the missile texture
      GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");
      for (int i = 0; i != num_missiles; ++i) {
        // create missiles off-screen
        sprites[first_missile_sprite + i].init(missile, 20, 0, 0.0625f, 0.25f, true);
        sprites[first_missile_sprite + i].is_enabled() = false;
      }

      // use the nuke texture
      // for now, same texture as missile
      for (int i = 0; i != num_nukes; ++i) {
        // create nukes off-screen
        sprites[first_nuke_sprite + i].init(missile, 20, 0, 0.2f, 0.5f, true, true, vec3(0.75f, 0.75f, 0.75f));
        sprites[first_nuke_sprite + i].is_enabled() = false;
      }

      // use the bomb texture
      GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/bomb.gif");
      for (int i = 0; i != num_bombs; ++i) {
        // create bombs off-screen
        sprites[first_bomb_sprite + i].init(bomb, 20, 0, 0.0625f, 0.25f, true, true, vec3(1.0f, 0.0f, 0.0f));
        sprites[first_bomb_sprite + i].is_enabled() = false;
      }

      // use the heal texture
      GLuint heal = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/life.gif");
      for (int i = 0; i != num_heals; ++i) {
        // create heals off-screen
        sprites[first_heal_sprite + i].init(heal, 20, 0, 0.4f, 0.4f, false, true, vec3(0, 1, 0));
        sprites[first_heal_sprite + i].is_enabled() = false;
      }

      // use the powerup texture
      GLuint powerup = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/star.gif");
      for (int i = 0; i != num_powerups; ++i) {
        // create powerups off-screen
        sprites[first_powerup_sprite + i].init(powerup, 20, 0, 0.4f, 0.4f);
        sprites[first_powerup_sprite + i].is_enabled() = false;
      }

      // use the nuke pickup texture
      // for now, same texture as nuke bullet
      for (int i = 0; i != num_nuke_pickups; ++i) {
        // create nukes off-screen
        sprites[first_nuke_pickup_sprite + i].init(missile, 20, 0, 0.15f, 0.375f, false, true, vec3(0.75f, 0.75f, 0.75f));
        sprites[first_nuke_pickup_sprite + i].is_enabled() = false;
      }

      //GUI sprites
      GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
      sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1.5f);
      GLuint win = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/you-win.gif");
      sprites[win_sprite].init(win, 20, 0, 5, 2);
      GLuint title = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invaderers.gif");
      sprites[title_sprite].init(title, 0.0f, 1.5f, 3.0f, 0.75f);
      GLuint normal_button = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/normal.gif");
      sprites[normal_button_sprite].init(normal_button, -1.5f, 0, 1.5f, 0.35f);
      GLuint hardcore_button = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/hardcore.gif");
      sprites[hardcore_button_sprite].init(hardcore_button, 1.5f, 0, 1.5f, 0.35f, false, true, vec3(0.5f, 0.5f, 0.5f));
      sprites[nukes_available_sprite].init(missile, 2.5f, 2.8f, 0.1f, 0.25f, false, true, vec3(0.75f, 0.75f, 0.75f));
      sprites[nukes_available_sprite].translate(20, 0);
      sprites[powerup_available_sprite].init(powerup, 2.15f, 2.8f, 0.175f, 0.175f);
      sprites[powerup_available_sprite].translate(20, 0);

      //background sprites
      for (int i = 0; i != num_background_sprites; ++i) {
        float scale = randomizer.get(0.25f, 1.0f);
        sprites[first_background_sprite + i].init(white, randomizer.get(-3.0f, 3.0f), randomizer.get(-3.0f, 3.0f), 0.0075f * scale, 0.04f * scale);
        sprites[first_background_sprite + i].is_enabled() = true;
      }
      
      // sounds
      whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/whoosh.wav");
      bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
      cling = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/cling.wav");
      tap = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/tap.wav");
      cur_source = 0;
      alGenSources(num_sound_sources, sources);

      // sundry counters and game state.
      missiles_disabled = 0;
      gamemode = MAIN_MENU;
      game_over = false;
      game_win = false;
      powerup_available = false;
      hardcore_unlocked = false;
      normal_highscore = 0;
      hardcore_highscore = 0;
      scene_velocity = -0.01f;

      process_csv_rows();
    }

    //Process the CSV file for row generation
    //Code copied and modified from: https://github.com/andy-thomason/read_a_csv_file/blob/master/main.cpp
    void process_csv_rows() {
      std::ifstream is("./invaderers.csv");
      if (is.good()) {
        char buffer[256]; // store the line here
        while (!is.eof()) { // loop over lines
          is.getline(buffer, sizeof(buffer));

          // loop over columns
          char *b = buffer;
          if (*b == '#') continue; //ignore commented lines
          std::vector<enum RowElement> row;
          for (int col = 0; ; ++col) {
            while (*b == ' ' || *b == '\t') ++b; //trim left whitespace

            switch (*b) {

            case 0:
            case ',':
            case '_':
              row.push_back(RowElement::None);
              break;

            case '+':
              row.push_back(RowElement::Heal);
              break;

            case '*':
              row.push_back(RowElement::Powerup);
              break;

            case 'o':
            case 'O':
            case 'n':
            case 'N':
              row.push_back(RowElement::Nuke);
              break;

            case 'b':
            case 'B':
              row.push_back(RowElement::Boss);
              break;

            default:
              row.push_back(RowElement::Invaderer);
            }

            while (*b != 0 && *b != ',') ++b; //trim right whitespace
            if (*b == ',') ++b;
            if (*b == 0) break;
          }

          rows.push_back(row);
        }
      }
    }

    // Resets the sprites and the game state to the main menu
    void reset_scene() {
      if (game_win) {
        game_win = false;
        sprites[win_sprite].translate(20, 0);
      }
      if (game_over) {
        game_over = false;
        sprites[game_over_sprite].translate(20, 0);
      }

      // entities
      for (int i = 0; i < num_invaderers; ++i) {
        sprite &invaderer = sprites[first_invaderer_sprite + i];
        if (invaderer.is_enabled()) {
          invaderer.translate(20, 0);
          invaderer.is_enabled() = false;
          available_invaderers.push_back(i);
        }
      }

      sprite &boss = sprites[boss_sprite];
      if (boss.is_enabled()) {
        boss.translate(20, 0);
        boss.is_enabled() = false;
      }

      //bullets
      for (int i = 0; i < num_missiles; ++i) {
        sprite &missile = sprites[first_missile_sprite + i];
        if (missile.is_enabled()) {
          missile.translate(20, 0);
          missile.is_enabled() = false;
        }
      }

      for (int i = 0; i < num_nukes; ++i) {
        sprite &nuke = sprites[first_nuke_sprite + i];
        if (nuke.is_enabled()) {
          nuke.translate(20, 0);
          nuke.is_enabled() = false;
        }
      }
      
      for (int i = 0; i < num_bombs; ++i) {
        sprite &bomb = sprites[first_bomb_sprite + i];
        if (bomb.is_enabled()) {
          bomb.translate(20, 0);
          bomb.is_enabled() = true;
        }
      }

      //pickups
      for (int i = 0; i < num_heals; ++i) {
        sprite &heal = sprites[first_heal_sprite + i];
        if (heal.is_enabled()) {
          heal.translate(20, 0);
          heal.is_enabled() = true;
        }
      }

      for (int i = 0; i < num_powerups; ++i) {
        sprite &powerup = sprites[first_powerup_sprite + i];
        if (powerup.is_enabled()) {
          powerup.translate(20, 0);
          powerup.is_enabled() = true;
        }
      }

      for (int i = 0; i < num_nuke_pickups; ++i) {
        sprite &nuke_pickup = sprites[first_nuke_pickup_sprite + i];
        if (nuke_pickup.is_enabled()) {
          nuke_pickup.translate(20, 0);
          nuke_pickup.is_enabled() = true;
        }
      }

      //GUI reset
      gamemode = MAIN_MENU;
      sprites[title_sprite].translate(-20, 0);
      sprites[normal_button_sprite].translate(-20, 0);
      sprites[hardcore_button_sprite].translate(-20, 0);
      sprites[nukes_available_sprite].translate(20, 0);
      if (powerup_available)
        sprites[powerup_available_sprite].translate(20, 0);

      // player stats
      nukes_available = 0;
      if (powerup_left > 0)
        powerup_left = 1;
      powerup_available = false;
      scene_velocity = -0.01f;
    }

    //Handles the selection of the main menu buttons and their actions
    void handle_main_menu() {
      if (gamemode != MAIN_MENU)
        return;

      for (int i = 0; i != num_missiles; ++i) {
        sprite &missile = sprites[first_missile_sprite + i];
        if (missile.is_enabled()) {
          if (missile.collides_with(sprites[normal_button_sprite])) {
            // Start normal mode
            gamemode = NORMAL;
            num_lives = 3;
            frames_between_rows = 40;
            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, cling);
            alSourcePlay(source);
            goto destroy_missile_and_break;
          }
          if (missile.collides_with(sprites[hardcore_button_sprite])) {
            if (hardcore_unlocked) {
              // Start hardcore mode
              gamemode = HARDCORE;
              scene_velocity = -0.02f;
              num_lives = 3;
              frames_between_rows = 20;
              boss_disabled = 0;
              ALuint source = get_sound_source();
              alSourcei(source, AL_BUFFER, cling);
              alSourcePlay(source);
            }
            goto destroy_missile_and_break;
          }
          if (false) {
          destroy_missile_and_break:;
            missile.is_enabled() = false;
            missile.translate(20, 0);
            break;
          }
        }
      }

      if (gamemode != MAIN_MENU) {
        missiles_disabled = 0;
        nukes_disabled = 0;
        bombs_disabled = 50;
        score = 0;
        counter_for_next_row = 0;
        current_row = 0;
        boss_lives = 0;
        nukes_available = 0;

        sprites[title_sprite].translate(20, 0);
        sprites[normal_button_sprite].translate(20, 0);
        sprites[hardcore_button_sprite].translate(20, 0);
        sprites[nukes_available_sprite].translate(-20, 0);
      }
    }

    // called every frame to move things
    void simulate() {
      if (game_over || game_win) {
        if (is_key_down(key_esc))
          reset_scene();
        return;
      }

      handle_main_menu();
      
      //Process timer for spawning a new row
      if ((gamemode == NORMAL || gamemode == HARDCORE) && --counter_for_next_row <= 0) {
        counter_for_next_row = frames_between_rows;

        spawn_new_row();
      }

      move_background(0, scene_velocity * 100.0f);

      update_powerup();

      move_ship();

      fire_missiles();

      fire_nukes();

      fire_bombs();

      move_pickups(0, scene_velocity);

      move_missiles();

      move_nukes();

      move_bombs();

      move_invaders(0, scene_velocity);

      move_boss(0, scene_velocity);
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      simulate();

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
      for (int i = 0; i != num_sprites; ++i) {
        sprites[i].render(texture_shader_, cameraToWorld);
      }

      // draw score text
      if (gamemode != MAIN_MENU) {
        char score_text[32];
        sprintf(score_text, "score: %d   lives: %d\n", score, num_lives);
        draw_text(texture_shader_, -1.75f, 2, 1.0f / 256, score_text);

        char nukes_text[8];
        sprintf(nukes_text, "x%d\n", nukes_available);
        draw_text(texture_shader_, 3.625f, 2.0f, 1.0f / 256, nukes_text);
      }

      if (game_over || game_win) {
        char highscore_text[32];
        sprintf(highscore_text, "Highest score: %d\n", gamemode == NORMAL ? normal_highscore : hardcore_highscore);
        draw_text(texture_shader_, 0.25f, 1, 1.0f / 256, highscore_text);

        char *help_text = "Press <Esc> to go back to the main menu.\n";
        draw_text(texture_shader_, 0, -2.5f, 1.0f / 320, help_text);
      }

      // move the listener with the camera
      vec4 &cpos = cameraToWorld.w();
      alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
    }
  };
}

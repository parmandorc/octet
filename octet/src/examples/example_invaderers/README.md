<h1>Introduction to programming assignment: Invaderers game.</h1>

<h2>Game mechanics</h2>
The game created has the basic mechanics of the original game, with several additions and changes. The focus of the project (apart from the basic requirements) has been gameplay logic programming.

<h4>Normal gamemode</h4>
This gamemode has a preset layout, specified in a CSV file that is read at start, that being the knowledge goal of this feature. At the begining of the CSV file exits a little manual with the expected format and how to spawn different elements in the scene.

<h4>Hardcore gamemode</h4>
This gamemode is unlocked once the player wins a normal game. The purpose of this gamemode is to show a random generation algorithm that builds the scene procedurally as oppossed to using a preset scene. It has also been implemented with increasing difficulty so it is considered as a challenge in the game.

<h4>New elements</h4>
Invaderers now take a life from the player when they pass them and reach the bottom of the screen.

One of the additions to the game is the Boss invaderer. This type of enemy has increased size, needs several hits to be killed, and kills the player instantly upon hitting or passing the player. As compensation, it is slower and does not fire against the player. Note also that there can only exist one boss at a time.

Pickups are another big addition the game:
- **Heals**. They grant an extra life to the player.
- **Powerup**. They grant the player to powerup upon pressing 'z', rendering inmune to bullet damage (not boss hit damage or invaderer passing), and granting them increased size and speed.
- **Nukes**. They grant the player the ability to fire an explosive bullet upon pressing 'x', which explodes on contact, killing all invaderers in a large area. Note that the Boss is not immediately killed upon nuke detonation, but takes considerable damage.

<h2>Fragment shader</h2>
The fragment shader has been modified in order to allow for extra features and ways to control the visuals of the scene. The way of passing inputs to the fragment shader is by the use of uniforms.

Firstly, sprites can be set to be rendered in *fake 3D*. This is not a real 3D rendering, since the game is 2D, but applies a central gradient to the sprites that make them seem to have a slight volume.

Secondly, sprites can be set to be rendered with a color tint, that will change the color of the sprite by reducing the unwanted color channels' values.

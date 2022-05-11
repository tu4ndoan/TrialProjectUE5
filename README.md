# Trial Project UE5 - TrialProjectUE5
trial project, unreal engine 5, cosu games

## Trial Project
- GameMode:
  - Team fight, 2 teams
  - In some defined time
  - If you kill an enemy, your team get score
  - If you hit an enemy, your hit count is recorded (for leaderboard)
  - hit event

- GameState:
  - handle score of 2 teams
  - handle hit count of player (show leaderboard in-game)
  - hit event?

- GameInstanceSubsystem:
  - interface for sessions...

- PlayerState:
  - keep player's information

- TrialProjectCharacter:
 - 2 attacks
 - simple health
 - sword (box collision component) used to generate overlap event when you hit someone
 - handle overlap event

## TODO:

  - Handle Player joins in-progress (sync animation, skeletal mesh, bone loc...)
  - Sphere Trace to hit sth

## Currently has:

 - 2 Simple Attacks (networked)

 - Create Session, Find Session, Join Session
 - Simple Health system
 - Add game mode, game state, player state,
 - total hit count
 - auto assign team when player join, still working on a simple damage system

## Working On:

 - Improve Damage system
 - Improve Health system

## current issue

 - AnimMontage Death and HitReact can't play

## For Testing

  - open editor, run dedicated server
  - open 2 client.bat
  - call open 127.0.0.1:17777 on each client.bat instance
  - test 2 clients, ignore the first client spawn by the editor
  - should build a dedicated server instead?

## end

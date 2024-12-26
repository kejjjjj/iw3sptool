# iw3sptool // a multipurpose tool for call of duty 4 singleplayer

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/0c3b108d-a0cf-4235-9138-0737ee0d4537)

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/d9c08f5e-47fc-4a8a-90e3-eb74a546b0f0)

## Features
- collision drawing
- map exporting 
- entity drawing
- thirdperson
- player movement modification
- demo playback

## Notes
- Only for cod4 singleplayer patch 1.5

## Usage
- The .asi file goes to the "miles" folder located in the cod4 root directory
- All commands start with the following prefixes:
    - cm_ (clip map)
    - pm_ (player movement)
    - cg_thirdPerson
- cm_showCollisionFilter expects the name of a material (e.g. "clip") and using it without an argument will clear the render queue
- cm_showEntities expects the name of a classname (e.g. "trigger_multiple") and using it without an argument will clear the render queue
- you can include multiple arguments on cm_showCollisionFilter and cm_showEntities by using a space-separated list, (e.g. "clip caulk mantle")
- using the "all" argument on cm_showCollisionFilter and cm_showEntities will select *ALL* relevant items
- brushes are highlighted with green
- terrain brushmodels are highlighted with purple
- terrain is highlighted with blue
- entities are highlighted with orange
- spawners are highlighted with red

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/75698826-97fb-4777-9cd7-b3da8009faf4)

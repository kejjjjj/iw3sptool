# iw3sptool // a tool to preview map surfaces

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/0c3b108d-a0cf-4235-9138-0737ee0d4537)

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/d9c08f5e-47fc-4a8a-90e3-eb74a546b0f0)


## Notes
- Only for cod4 singleplayer patch 1.5
- you MIGHT crash the game if you disconnect when there are a lot of surfaces to draw so I guess clear the render queue first and then disconnect :)

## Usage
- The .asi file goes to the "miles" folder located in the cod4 root directory
- All commands start with the cm_ prefix (clip map)
- cm_showCollisionFilter expects the name of a material (e.g. "clip") and using it without an argument will clear the render queue
- cm_showEntities expects the name of a classname (e.g. "trigger_multiple") and using it without an argument will clear the render queue
- using the "all" argument on cm_showCollisionFilter and cm_showEntities will select *ALL* relevant items 

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/75698826-97fb-4777-9cd7-b3da8009faf4)

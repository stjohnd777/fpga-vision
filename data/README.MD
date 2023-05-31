# Start the Node Server

```
export PATH=/run/media/mmcblk0p1/data/nodejs/node-v19.9.0-linux-arm64/bin:$PATH
cd /run/media/mmcblk0p1/data/nodejs/app/simple
node bin/www &
```

## start node server
```
export PATH=/run/media/mmcblk0p1/data/nodejs/node-v19.9.0-linux-arm64/bin:$PATH && cd /run/media/mmcblk0p1/data/nodejs/app/simple && node bin/www &
```

## invoke sgs
```
./optxclient gerolamo.ast.lmco.com 2121  ./scene_config.jsonc ./nodejs/app/simple/public/sgs/sgs_image_0.png
./optxclient gerolamo.ast.lmco.com 2121  ./scene_iss.jsonc ./nodejs/app/simple/public/sgs/sgs_image_1.png
```
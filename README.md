## Lancer Bellepoule avec Docker

Pour faire fonctionner Bellepoule avec Docker : 

1) Installer WSL : https://learn.microsoft.com/fr-fr/windows/wsl/install
2) Installer Docker Desktop (en activant l'intégration wsl) : https://www.docker.com/products/docker-desktop/
3) Lire la documentation WSLg officielle (notamment pour les pilotes et environnements supportés) : https://github.com/microsoft/wslg et https://github.com/microsoft/wslg/blob/main/samples/container/Containers.md
4) Lancer le conteneur docker en modifiant `<data>` par l'emplacement des données souhaité et `<date>` par le tag de la dernière date de build : 
```bash
sudo docker run -it -d -v /tmp/.X11-unix:/tmp/.X11-unix -v /mnt/wslg:/mnt/wslg -e DISPLAY=$DISPLAY -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY     -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR -e PULSE_SERVER=$PULSE_SERVER -p 8080:8080 -p 8000:8000 -v <data>:/root/ farfeduc/bellepoule:beta-custom-<date>
```
Pour trouver la dernière date de build, vous pouvez aller sur cette page : https://hub.docker.com/r/farfeduc/bellepoule/tags
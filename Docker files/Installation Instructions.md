# Installation Instructions

1. Install the latest version of Docker
   - [Windows](https://docs.docker.com/docker-for-windows/install/)
   - [Mac](https://docs.docker.com/docker-for-mac/install/)
2. Run `docker pull systemsandnetworks/cs2200-students`
3. Open the docker desktop application and let it **run in the background**
4. Type either of the following into your terminal (These commands must be run in the extracted `docker` folder):
   - On Windows: run `.\run.bat` if using powershell or just `run.bat` if using command prompt
   - On Mac and Linux, first run `chmod +x run.sh` then `./run.sh`

If step 4 works correctly, you should see something like this:

```
root@57a4f8b9fe5b:/cs2200#
```

You can close the container by typing `exit` or pressing `Ctrl+D`. You can start the container again by running the same command as in step 4.

## How the workspace folder works

Everything that is put in the workspace folder is shared between your computer and the docker container. You should put your homework and project files in this folder when you use the container.

## Opening additional terminals

To open additional terminals for the same docker session, use the attach script we gave you in a separate terminal. Execute it the same way as the run.bat/run.sh script (including running `chmod` on it)

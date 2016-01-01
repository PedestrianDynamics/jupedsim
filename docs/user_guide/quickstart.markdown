# Get started

After succesfully [compiling](configuration) the code you can just run
`jpscore` in a Terminal as follows:

```
./bin/jpscore inifile.xml
```

*inifile.xml* defines all the needed information to start a simulation with
`jpscore`. See  the specifications of the [file format](data_formats).

After a successful simulation an additional xml-file will be created in
the same directory as the used inifile. It contains discrete values
for the trajectories of the agents.

You can visualize the produced trajectory file with `jpscore` as follows

```
/path/to/jpsvis  trajectory.xml
```

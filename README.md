# nanoinit
Linux based, Docker-aimed supervisor with very low memory footprint and full control of targeted applications.

nanoinit is a dependency-free, very lightweight supervisor application (daemon) especially crafted for using inside Docker containers.
## Features
Disk size:
- for **Ubuntu Linux**: ~41KB
- for **Alpine Linux**: ~66KB

RAM size:
- as little as possible, dependent on the number of apps in the config file; usually **does not exceed a few KB**.

### Main features
- JSON configuration via the **-c** argument; see [arguments](#arguments) for more information
- non-dedicated JSON configuration via the **-j** argument; see [arguments](#arguments) for more information
- save logs to your desired path; see [config file](#config) for more information
- add any number of apps to supervise, with any combination of parameters; see [config file](#config) for more information
- redirect stdout and/or stderr of your applications to specific locations; see [config file](#config) for more information
- autorestart failed apps; see [config file](#config) for more information
- manual mode for specific apps; [arguments](#arguments) and [config file](#config) for more information

### Manual mode
Applications marked as manual in the config file won't be ran (whole entry is ignored) if nanoinit runs in manual mode. Running nanoinit in manual mode can be done either by using the **-m** argument (see [arguments](#arguments)) or by setting the **NANOINIT_MANUAL_MODE** environment variable to anything non-null (see [environment variables](#envvars)).

Manual mode has advantages when debugging applications and wanting to manually run them inside the container.

The preffered method of activating manual mode is through environment variables, since custom env vars can be specified directly to Docker when running an image.

### stdout / stderr redirection
stdout and stderr redirection can be configured for each application through the [config file](#config).

## Requirements
- for **Ubuntu Linux**: none
- for **Alpine Linux**: argp-standalone

## Install
Add the following directive to your Dockerfile:
```
COPY nanoinit /path/to/nanoinit
```

## Usage
Add the following command at the end of your Dockerfile:
```
CMD ["/path/to/nanoinit", "--config-file=/path/to/config.json", "--config-json-object=/nanoinit-settings", "--log-path=/path/to/save/logs"]

which is the same as

CMD ["/path/to/nanoinit", "-c/path/to/config.json", "-j/nanoinit-settings", "-l/path/to/save/logs"]
```

See [arguments](#arguments), [config file](#config) and [environment variables](#envvars) below for more information.
## Arguments
Arguments are passed to the nanoinit binary when it is ran.

**Note:** [environment variables](#envvars) **superseed** arguments when there's a conflict.

### -c, --config-file=/path/to/config.json
Specifies the configuration JSON file.

If argument is not specified, default value is null, which means:
- no configuration is loaded/available
- no apps will be run, but nanoinit will sleep for infinity and wait for a kill signal

### -j, --config-json-object=nanoinit-settings
Specifies the parent JSON object.

Default value is null, which means that it will look directly into the root of the JSON file.

The JSON object must be specified as a path. For example:
- **/nanoinit-rules** if the configuration object is in the root of the JSON, under the "nanoinit-rules" object.
- **/config/nanoinit** if the configuration object is inside the "config" object, which is in the root of the JSON file.

### -l, --log-path=/path/to/log.txt
Specified the path for writing log-files.

Default only uses stderr and stdout for logging. When specified, stdout and stderr are still outputed, but the output is also written to a certain file (stdout and stderr combined).

### -m, --manual-mode
Enable manual mode. 

Default is manual-mode disabled.

This option is recommended to be set via the NANOINIT_MANUAL_MODE environment variable, as it is more useful that way.

### -r, --reload
Looks for top nanoinit process and sends a SIGSUSR1 signal to it, forcing it to terminate all apps, reload config and restart apps.

### -v, --verbose=0-2
Specified application print verbosity level.

Values are 0(nanoinit ERR), 1(application ERR), 2(LOG), and default value is 0.

## Config file
Configuration file is in JSON format, parsed by the [edJSON library](https://github.com/AXIPlus/edJSON). This way, JSON parsing is very lightweight, highly efficient, and **JSON comments** are enabled.

The complete set of parameters for one application:
```
"application_name": {
    "path": "/path/to/app_binary",
    "args": ["-a", "-b"],
    "autorestart": true,
    "manual": false,
    "stdout": "log/program1-stdout.log",
    "stderr": "log/program1-stderr.log"
},
```
All paths are relative to **nanoinit**'s working directory.

Parameters:
- **path** - specified the path to the application to be ran
- **args** - arguments to be passed to the application; can be a string if only one argument is present, or an array of arguments for multiple arguments; setting more than one argument in one string may lead to undefined behaviour;
- **autorestart** - whether to restart the app automatically when it exists or not; default value is **false**;
- **manual** - whether the application is marked as manual or not; default value is **false**;
- **stdout** and **stderr** - used to redirect app's output streams stdout and stderr; default value is **unset**
    - when **unset**, nanoinit does not redirect the stream, which are outputted
    - when set to **/a/path/on/disk** the stream is redirected to the specified path
    - when set to **empty** ("") the stream is redirected to /dev/null

Besides **path**, all other parameters are optional.

### Config file examples
Below is an example config.json file when the file is dedicated to nanoinit (JSON object is **not set**):
```
{
    "program1": {
        "path": "/path/to/program1",
        "args": ["-a", "-b"],
        "autorestart": true,
        "stdout": "log/program1-stdout.log",
        "stderr": "log/program1-stderr.log"
    },

    "program2": {
        "path": "/path/to/program2",
        "args": ["-p/a/path", "--path=/another/path"],
        "manual": true
    },

    "program3": {
        "path": "/path/to/program3",
        "args": ["-p/a/path", "--path=/another/path"],
        "stdout": "",
        "stderr": ""
    },

    "program4": {
        "path": "/path/to/program3",
        "args": "-ahl"
    }
}
```

Below is an example config.json when the file is shared with other applications (JSON object is **'/nanoinit-rules'**):
```
{
    "nanoinit-rules": {
        "program1": {
            "path": "/path/to/program1",
            "args": ["-a", "-b"],
            "autorestart": true,
            "stdout": "log/program1-stdout.log",
            "stderr": "log/program1-stderr.log"
        },

        "program2": {
            "path": "/path/to/program2",
            "args": ["-p/a/path", "--path=/another/path"],
            "manual": true
        },

        "program3": {
            "path": "/path/to/program3",
            "args": ["-p/a/path", "--path=/another/path"],
            "stdout": "",
            "stderr": ""
        },

        "program4": {
            "path": "/path/to/program3",
            "args": "-ahl"
        }
    },

    //below, any other objects are ignored by nanoinit
    "other": {

    },

    "other2": {

    }
}
```

## Environment variables
Enviroment variables can be specified via Docker run command to change the behaviour of nanoinit on the go:

- **NANOINIT_MANUAL_MODE**: sets manual mode (for app-debugging purposes)
- **NANOINIT_CONFIG_FILE**: sets config file, if a different config file than the one specified in the Dockerfile needs to be used (for app-debugging purposes)
- **NANOINIT_CONFIG_JSON_OBJECT**: sets the config object, if a different config object than the one specified in the Dockerfile is used (for app-debugging purposes)


## Release notes
### version 1.0.0
- initial release

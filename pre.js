Module.preRun = () => {
    const PREFIX = "/share/ucblogo";
    ENV.LOGOLIB = PREFIX+"/logolib";
    ENV.LOGOHELP = PREFIX+"/helpfiles";
    ENV.CSLS = PREFIX+"/csls";
};
Module.env = {};
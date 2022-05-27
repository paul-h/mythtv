import { Encoder } from "./encoder.interface";
import { JobQueueJob } from "./jobqueue.interface";
import { ScheduleOrProgram } from "./program.interface";
import { StorageGroup } from "./storagegroup.interface";
import { Backend } from "./backend.interface";
import { Frontend } from "./frontend.interface";

export interface MachineInfo {
    GuideDays:      number;
    GuideEnds:      string;
    GuideNext:      string;
    GuideStart:     string;
    GuideStatus:    string;
    GuideThru:      string;
    LoadAvg1:       string;
    LoadAvg2:       string;
    LoadAvg3:       string;
    StorageGroups:  StorageGroup[];
}

export interface BackendStatus {
    AsOf:           string;
    Backends:       Backend[];
    Encoders:       Encoder[];
    Frontends:      Frontend[];
    JobQueue:       JobQueueJob[];
    MachineInfo:    MachineInfo;
    Miscellaneous:  string;
    ProtoVer:       string;
    Scheduled:      ScheduleOrProgram[];
    Version:        string;
}

export interface BackendStatusResponse {
    BackendStatus:  BackendStatus;
}
declare interface IPAAFixedStatus {
  success: boolean;
  data: {
    flag: boolean;
  };
}

declare interface IPAAFixedUpdateRunningStatus {
  success: boolean;
  data: boolean;
}

declare interface IPAAFixedStopResponse {
  success: boolean;
  message: string;
}

declare interface IPAAFixedStartResponse {
  success: boolean;
  message: string;
}

// --- request declare

declare interface IPAAFixedGotoRequest {
  ra: number;
  dec: number;
}

declare interface IPAAFixedPolarAlignmentRequest {
  start_from: "West" | "East";
  move_time: number;
  solve_retry: number;
  manual_start?: boolean;
  search_radius?: number;
}

declare interface IPAAFixedAutofocusRequest {
  filter_index: "current" | number;
  start_side: boolean;
}

// PPA scripts

declare interface PAAEmptyProps {}

declare interface PAAScriptBaseHanlder {
  savePAAScript: () => void;
}

declare interface TargetSelectDialogHandler {
  open_dialog: () => void;
}

declare interface PAA_step_info {
  data: {
    params?: {
      count: number;
    };
  };
  device_name: string;
  instruction: string;
  message: string;
  type: string;
}

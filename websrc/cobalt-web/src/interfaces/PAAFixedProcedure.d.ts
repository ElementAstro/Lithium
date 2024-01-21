
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

declare interface IPAAFixedPolarAlignmentRequest{
  start_from: 'West' | 'East';
  move_time: number;
  solve_retry: number;
  manual_start?: boolean;
}


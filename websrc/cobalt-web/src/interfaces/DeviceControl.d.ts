declare interface IndiNumberStruct {
  name: string;
  label: string;
  value: number;
  max: number;
  min: number;
}

declare interface IndiTextStruct {
  name: string;
  label: string;
  text: string;
}

declare interface IndiSwitchStruct {
  name: string;
  label: string;
  switch: boolean;
}

declare interface IndiBlobStruct {
  name: string;
  label: string;
  size: number;
}

declare interface IndiLightStruct {
  name: string;
  label: string;
  status: string;
}

declare interface IndiPropertyNumberDataStruct {
  property_name: string;
  property_info: {
    type: "NUMBER";
    data: Array<IndiNumberStruct>;
    permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  };
}
declare interface IndiPropertyTextDataStruct {
  property_name: string;
  property_info: {
    type: "TEXT";
    data: Array<IndiTextStruct>;
    permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  };
}
declare interface IndiPropertySwitchDataStruct {
  property_name: string;
  property_info: {
    type: "SWITCH";
    data: Array<IndiSwitchStruct>;
    permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  };
}
declare interface IndiPropertyBlobDataStruct {
  property_name: string;
  property_info: {
    type: "BLOB";
    data: Array<IndiBlobStruct>;
    permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  };
}
declare interface IndiPropertyLightDataStruct {
  property_name: string;
  property_info: {
    type: "LIGHT";
    data: Array<IndiLightStruct>;
    permission: "ReadOnly" | "WriteOnly" | "ReadWrite";
  };
}

declare type IndiPropertyGroupStruct =
  | IndiPropertyNumberDataStruct
  | IndiPropertyTextDataStruct
  | IndiPropertySwitchDataStruct
  | IndiPropertyBlobDataStruct
  | IndiPropertyLightDataStruct;

declare interface IndiLabelDescription {
  name: string;
  show_name: string;
  tooltips: string;
}

declare interface IndiSingleDeviceProfile {
  device_type: string;
  device_name: string;
}

declare interface IndiConnectProfile {
  data: any;
  all_drivers: [];
  telescope: IndiSingleDeviceProfile;
  camera: IndiSingleDeviceProfile;
  focuser: IndiSingleDeviceProfile;
  filter: IndiSingleDeviceProfile;
  guide_camera: IndiSingleDeviceProfile;
  polar: IndiSingleDeviceProfile;
}

interface HelperHandle {
  open_snackbar: () => void;
}

declare interface DeviceCustomSwitchValue {
  custom_name: string;
  display_label: string;
  data: boolean;
  data_type: "SWITCH";
}

declare interface DeviceCustomSelectValue {
  custom_name: string;
  display_label: string;
  data: {
    selections: string[];
    selected: string;
  };
  data_type: "SELECT";
}

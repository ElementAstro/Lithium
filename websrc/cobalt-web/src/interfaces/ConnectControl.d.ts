declare interface ConnectionDeviceInfo {
  device_name: string;
  device_driver_name: string;
  device_driver_exec: string;
}

declare interface IConnectBrandSelection {
  zh: string;
  en: string;
  driver: string;
}

declare interface IConnectBrandList {
  camera: Array<IConnectBrandSelection>;
  telescope: Array<IConnectBrandSelection>;
  guider: Array<IConnectBrandSelection>;
  focus: Array<IConnectBrandSelection>;
  filter: Array<IConnectBrandSelection>;
  polar?: Array<IConnectBrandSelection>;
}

declare interface IConnectSelectedBrandList {
  camera: IConnectBrandSelection;
  telescope: IConnectBrandSelection;
  guider: IConnectBrandSelection;
  focus: IConnectBrandSelection;
  filter: IConnectBrandSelection;
  polar?: IConnectBrandSelection;
}

declare interface IConnectSelectedDeviceList {
  camera: ConnectionDeviceInfo;
  telescope: ConnectionDeviceInfo;
  guider: ConnectionDeviceInfo;
  focus: ConnectionDeviceInfo;
  filter: ConnectionDeviceInfo;
  polar?: ConnectionDeviceInfo;
}

declare interface IConnectSupportedDevicesNameENType {
  camera: null;
  telescope: null;
  guider: null;
  focus: null;
  filter: null;
  polar: null;
}

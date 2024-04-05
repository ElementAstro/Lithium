declare interface ConnectionDeviceInfo {
  device_type: string;
  device_name: string;
}

declare interface IConnectBrandSelection {
  zh: string;
  en: string;
  driver: string;
}

declare interface IConnectSelectedBrandList {
  camera: IConnectBrandSelection;
  telescope: IConnectBrandSelection;
  guider: IConnectBrandSelection;
  focus: IConnectBrandSelection;
  filter: IConnectBrandSelection;
  polar?: IConnectBrandSelection;
}

declare interface IConnectSupportedDevicesNameENType {
  camera: null;
  telescope: null;
  guider: null;
  focus: null;
  filter: null;
  polar: null;
}

declare type DeepPartial<T extends object> = Partial<{
  [k in keyof T]: T[k] extends object ? DeepPartial<T[k]> : T[k];
}>;

declare type IProps = {
  children?: React.ReactNode;
};

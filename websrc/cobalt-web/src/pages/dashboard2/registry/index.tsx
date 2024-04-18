import * as React from "react";
import { Android } from "react-bootstrap-icons";
import { IconProps } from "react-bootstrap-icons";

class Registry {
  static _registry: Map<string, React.FC> = new Map();
  static _registry_icon: Map<string, React.FC<IconProps>> = new Map();

  static addComponent = (
    id: string,
    component: React.FC,
    Icon: React.FC<IconProps> | null = null
  ) => {
    this._registry.set(id, component);
    if (Icon === null) {
      this._registry_icon.set(id, (props: IconProps) => <Android {...props} />);
    } else {
      this._registry_icon.set(id, Icon);
    }
  };

  static getComponent = (id: string) => this._registry.get(id);

  static getMenus = () => {
    return Array.from(this._registry.keys()).map((id) => ({
      id,
      displayName: this._registry.get(id)?.displayName,
      icon: (props: IconProps) => {
        const Icon = this._registry_icon.get(id);
        return (
          <div style={{ width: 24, height: 24 }}>
            <Icon {...props} style={{ width: "100%", height: "100%" }} />
          </div>
        );
      },
    }));
  };
}

export default Registry;

import * as React from "react";
import { Android } from "react-bootstrap-icons";

class Registry {
  static _registry: Map<string, React.FC> = new Map();
  static _registry_icon: Map<string, React.FC> = new Map();

  static addComponent = (
    id: string,
    component: React.FC,
    Icon: React.FC | null = null
  ) => {
    this._registry.set(id, component);
    if (Icon === null) {
      this._registry_icon.set(id, () => <Android></Android>);
    } else {
      this._registry_icon.set(id, () => {
        return (
          <div style={{ width: 24, height: 24 }}>
            {React.createElement(Icon, {
              style: { width: "100%", height: "100%" },
              fill: "#fff",
            })}
          </div>
        );
      });
    }
  };
  static getComponent = (id: string) => this._registry.get(id);

  static getMenus = () => {
    return Array.from(this._registry.keys()).map((id) => ({
      id,
      displayName: this._registry.get(id)?.displayName,
      icon: this._registry_icon.get(id) as React.FC,
    }));
  };
}

export default Registry;

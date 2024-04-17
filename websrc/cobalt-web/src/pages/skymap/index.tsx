import * as React from "react";
import { Card } from "react-bootstrap";
import { GlobalStore } from "../..//store/globalStore";
import { Tabs, Tab } from "react-bootstrap";

//import ObjectSearch from './ObjectSearch';
import ImageFraming from "./image_framing";
//import ObjectManagement from './ObjectManagement';
///import TargetTestPage from './TargetTestPage';

const ObjectFinding: React.FC = () => {
  // reload all global parameter first.
  const get_all_global_parameters =
    GlobalStore.actions.GlobalParameterStore.get_all_paramters;
  React.useEffect(() => {
    get_all_global_parameters();
  }, []);

  return (
    <Card className="w-100">
      <ImageFraming />
    </Card>
    // <Tabs defaultActiveKey="search">
    //   <Tab eventKey="search" title="目标搜索">
    //     <ObjectSearch />
    //   </Tab>
    //   <Tab eventKey="framing" title="构图助手">
    //     <ImageFraming />
    //   </Tab>
    //   <Tab eventKey="management" title="拍摄目标管理">
    //     <ObjectManagement />
    //   </Tab>
    //   <Tab eventKey="test" title="测试页面">
    //     <TargetTestPage />
    //   </Tab>
    // </Tabs>
  );
};

export default ObjectFinding;

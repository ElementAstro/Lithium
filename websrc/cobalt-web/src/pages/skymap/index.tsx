import React from "react";
import { Tabs, Tab } from "react-bootstrap";
import {
  Search,
  Grid3x3,
  ClipboardData,
  Code
} from "react-bootstrap-icons";
import ObjectSearch from "./object_search";
import ImageFraming from "./image_framing";
import ObjectManagement from "./object_manager";
//import TargetTestPage from './TargetTestPage';

const ObjectFinding = () => {
  return (
    <Tabs defaultActiveKey="imageFraming" id="objectFindingTabs">
      <Tab
        eventKey="objectSearch"
        title={
          <>
            <Search /> 目标搜索
          </>
        }
      >
        <ObjectSearch on_choice_maken={null} />
      </Tab>
      <Tab
        eventKey="imageFraming"
        title={
          <>
            <Grid3x3 /> 构图助手
          </>
        }
      >
        <ImageFraming />
      </Tab>
      <Tab
        eventKey="objectManagement"
        title={
          <>
            <ClipboardData /> 拍摄目标管理
          </>
        }
      >
        <ObjectManagement on_choice_maken={null} />
      </Tab>
    </Tabs>
  );
};

export default ObjectFinding;

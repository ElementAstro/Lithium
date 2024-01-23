import React, { useState } from "react";
import { Container } from "react-bootstrap";
import axios from "axios";
import ToolBar from "../../components/module/toolbar";
import ModuleCard from "../../components/module/card";
import ModuleList from "./list";

function ModuleManager(props) {
  const [moduleList, setModuleList] = useState([
    {
      id: 1,
      name: "Module 1",
      author: "Author 1",
      version: "1.0.0",
      thumbnail: "path/to/thumbnail1.jpg",
      package: {
        name: "module-1",
        dependencies: {
          react: "^17.0.2",
          axios: "^0.24.0",
        },
      },
    },
    {
      id: 2,
      name: "Module 2",
      author: "Author 2",
      version: "2.0.0",
      thumbnail: "path/to/thumbnail2.jpg",
      package: {
        name: "module-2",
        dependencies: {
          react: "^17.0.2",
          lodash: "^4.17.21",
        },
      },
    },
    {
      id: 3,
      name: "Module 3",
      author: "Author 3",
      version: "3.0.0",
      thumbnail: "path/to/thumbnail3.jpg",
      package: {
        name: "module-3",
        dependencies: {
          react: "^17.0.2",
          moment: "^2.29.1",
        },
      },
    },
  ]);

  const handleRefresh = () => {
    // 点击刷新按钮时重新获取模组列表
    fetchModules();
  };

  const fetchModules = () => {
    // 向后端发送请求获取模组列表
    axios.get("/api/modules").then((res) => {
      setModuleList(res.data);
    });
  };

  const handleEnable = (id) => {
    // 点击启用按钮时向后端发送请求启用对应的模组
    axios.put(`/api/modules/${id}/enable`).then((res) => {
      fetchModules();
    });
  };

  const handleDisable = (id) => {
    // 点击停用按钮时向后端发送请求停用对应的模组
    axios.put(`/api/modules/${id}/disable`).then((res) => {
      fetchModules();
    });
  };

  const handleDelete = (id) => {
    // 点击删除按钮时向后端发送请求删除对应的模组
    axios.delete(`/api/modules/${id}`).then((res) => {
      fetchModules();
    });
  };

  const handleDragEnd = (result) => {
    if (!result.destination) return;

    const items = Array.from(moduleList);
    const [reorderedItem] = items.splice(result.source.index, 1);
    items.splice(result.destination.index, 0, reorderedItem);

    setModuleList(items);
  };

  return (
    <Container>
      <ToolBar onRefresh={handleRefresh} />
      <ModuleList onDragEnd={handleDragEnd}>
        {moduleList.map((module) => (
          <ModuleCard
            key={module.id}
            module={module}
            onEnable={handleEnable}
            onDisable={handleDisable}
            onDelete={handleDelete}
          />
        ))}
      </ModuleList>
    </Container>
  );
}

export default ModuleManager;

import React, { useState } from "react";
import { Container, FormControl } from "react-bootstrap";
import axios from "axios";
import ToolBar from "./toolbar";
import ModuleCard from "./card";
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
      license: "MIT",
      description: "This is a module description.",
      package: {
        name: "module-2",
        dependencies: {
          react: "^17.0.2",
          lodash: "^4.17.21",
        },
      },
      packageJson: {
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
      packageJson: {
        name: "module-3",
        dependencies: [
          "react", "moment",
        ]
      },
    },
  ]);
  const [searchText, setSearchText] = useState("");
  const [sortField, setSortField] = useState("name");
  const [sortOrder, setSortOrder] = useState("asc");

  const handleRefresh = () => {
    fetchModules();
  };

  const fetchModules = () => {
    axios
      .get("/api/modules", {
        params: {
          searchText,
          sortField,
          sortOrder,
        },
      })
      .then((res) => {
        setModuleList(res.data);
      });
  };

  const handleEnable = (id) => {
    axios.put(`/api/modules/${id}/enable`).then((res) => {
      fetchModules();
    });
  };

  const handleDisable = (id) => {
    axios.put(`/api/modules/${id}/disable`).then((res) => {
      fetchModules();
    });
  };

  const handleDelete = (id) => {
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

  const handleSearchChange = (e) => {
    setSearchText(e.target.value);
  };

  const handleSortChange = (e) => {
    const [field, order] = e.target.value.split(".");
    setSortField(field);
    setSortOrder(order);
  };

  return (
    <Container>
      <ToolBar
        onRefresh={handleRefresh}
        onSearchChange={handleSearchChange}
        onSortChange={handleSortChange}
      />
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

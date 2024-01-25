import React, { useEffect, useState } from "react";
import { Card, Button, Form } from "react-bootstrap";
import { GearFill, ArrowUpCircleFill, Search } from "react-bootstrap-icons";
import axios from "axios";

import styled from "styled-components";

const ScrollToTopButton = styled.div`
  position: fixed;
  bottom: 20px;
  right: 20px;
  cursor: pointer;
`;

const ScrollToTopIcon = styled(ArrowUpCircleFill)`
  font-size: 32px;
`;

const SearchBar = styled.div`
  display: flex;
  align-items: center;
  margin-bottom: 20px;

  & > *:not(:last-child) {
    margin-right: 10px;
  }
`;

const SearchInput = styled(Form.Control)`
  width: 200px;
`;

const SearchResult = styled.div`
  margin-top: 10px;
`;

interface ConfigData {
  [key: string]: {
    [key: string]: {
      value: string;
      type: string;
      validation?: {
        required?: boolean;
        minLength?: number;
        maxLength?: number;
        minValue?: number;
        maxValue?: number;
        pattern?: string;
        errorMessage?: string;
      };
    };
  };
}

const ConfigManager = () => {
  const [configData, setConfigData] = useState<ConfigData>({
    category1: {
      key1: {
        value: "default value 1",
        type: "text",
      },
      key2: {
        value: "default value 2",
        type: "number",
      },
    },
    category2: {
      key3: {
        value: "default value 3",
        type: "text",
      },
      key4: {
        value: "default value 4",
        type: "text",
      },
    },
  });
  const [showScrollToTop, setShowScrollToTop] = useState(false);
  const [searchTerm, setSearchTerm] = useState("");
  const [searchResults, setSearchResults] = useState([]);

  // 从后端获取配置数据
  useEffect(() => {
    axios
      .get("/api/config")
      .then((response) => {
        setConfigData(response.data);
      })
      .catch((error) => {
        console.error("获取配置数据出错:", error);
      });

    // 监听页面滚动事件
    const handleScroll = () => {
      if (window.scrollY > 200) {
        setShowScrollToTop(true);
      } else {
        setShowScrollToTop(false);
      }
    };

    window.addEventListener("scroll", handleScroll);
    return () => {
      window.removeEventListener("scroll", handleScroll);
    };
  }, []);

  const handleSetConfig = (category: string, name: string, value: string) => {
    axios
      .post("/api/config", {
        category,
        name,
        value,
      })
      .then((response) => {
        console.log(`设置配置成功: ${response.data}`);
      })
      .catch((error) => {
        console.error(`设置配置失败: ${error}`);
      });
  };

  const handleInputChange = (
    category: string,
    name: string,
    e: React.ChangeEvent<HTMLInputElement>
  ) => {
    const value = e.target.value;
    setConfigData((prevData) => ({
      ...prevData,
      [category]: {
        ...prevData[category],
        [name]: {
          ...prevData[category][name],
          value,
        },
      },
    }));
  };

  const validateInput = (
    value: string,
    validation?: ConfigData[keyof ConfigData][keyof ConfigData[keyof ConfigData]]["validation"]
  ) => {
    if (validation) {
      if (validation.required && value.trim() === "") {
        return validation.errorMessage || "该字段不能为空";
      }
      if (validation.minLength && value.length < validation.minLength) {
        return (
          validation.errorMessage ||
          `该字段长度不能小于 ${validation.minLength}`
        );
      }
      if (validation.maxLength && value.length > validation.maxLength) {
        return (
          validation.errorMessage ||
          `该字段长度不能大于 ${validation.maxLength}`
        );
      }
      if (validation.minValue && Number(value) < validation.minValue) {
        return (
          validation.errorMessage || `该字段值不能小于 ${validation.minValue}`
        );
      }
      if (validation.maxValue && Number(value) > validation.maxValue) {
        return (
          validation.errorMessage || `该字段值不能大于 ${validation.maxValue}`
        );
      }
      if (validation.pattern && !new RegExp(validation.pattern).test(value)) {
        return validation.errorMessage || "该字段格式不正确";
      }
    }
    return "";
  };

  const renderInputField = (
    category: string,
    name: string,
    config: ConfigData[keyof ConfigData][keyof ConfigData[keyof ConfigData]]
  ) => {
    const { value, type, validation } = config;
    const error = validateInput(value, validation);

    return (
      <div
        key={name}
        className="d-flex align-items-center justify-content-between mb-2"
        ref={(el) => {
          if (searchTerm && el && el.textContent?.includes(searchTerm)) {
            setSearchResults((prevResults) => [
              ...prevResults,
              el as HTMLDivElement,
            ]);
          }
        }}
      >
        <div className="config-name">{name}</div>
        <div className="d-flex align-items-center">
          <div className="config-value mr-2">{value}</div>
          <Form.Control
            type={type}
            className={`config-input mr-2 ${error ? "is-invalid" : ""}`}
            value={value}
            onChange={(e) => handleInputChange(category, name, e)}
          />
          {error && <div className="invalid-feedback">{error}</div>}
          <Button
            variant="primary"
            onClick={() => handleSetConfig(category, name, value)}
          >
            <GearFill size={18} />
          </Button>
        </div>
      </div>
    );
  };

  const handleScrollToTop = () => {
    window.scrollTo({
      top: 0,
      behavior: "smooth",
    });
  };

  const handleSearch = (e: React.ChangeEvent<HTMLInputElement>) => {
    setSearchTerm(e.target.value);
    setSearchResults([]);
  };

  useEffect(() => {
    if (searchTerm) {
      setSearchResults([]);
      // 搜索匹配结果
      const matchingResults = Array.from(
        document.querySelectorAll(".config-name")
      ).filter((el) => el.textContent?.includes(searchTerm));
      setSearchResults(matchingResults);
      // 将第一个匹配项滚动到可视区域内
      if (matchingResults.length > 0) {
        matchingResults[0].scrollIntoView({
          behavior: "smooth",
          block: "center",
        });
      }
    }
  }, [searchTerm]);

  return (
    <>
      <h1 className="title">配置管理</h1>
      <SearchBar>
        <Search size={20} />
        <SearchInput
          type="text"
          placeholder="搜索配置项"
          value={searchTerm}
          onChange={handleSearch}
        />
      </SearchBar>
      {searchResults.length > 0 && (
        <SearchResult>
          共找到 {searchResults.length} 个匹配结果：
          {searchResults.map((el, index) => (
            <div
              key={index}
              className="d-inline-block ml-2 text-primary"
              onClick={() =>
                el.scrollIntoView({ behavior: "smooth", block: "center" })
              }
            >
              {el.textContent}
            </div>
          ))}
        </SearchResult>
      )}
      {Object.entries(configData).map(([category, configs]) => (
        <Card key={category} className="mb-3">
          <Card.Header className="d-flex align-items-center justify-content-between">
            {category}
          </Card.Header>
          <Card.Body>
            {Object.entries(configs).map(([name, config]) =>
              renderInputField(category, name, config)
            )}
          </Card.Body>
        </Card>
      ))}
      {showScrollToTop && (
        <ScrollToTopButton onClick={handleScrollToTop}>
          <ScrollToTopIcon />
        </ScrollToTopButton>
      )}
    </>
  );
};

export default ConfigManager;

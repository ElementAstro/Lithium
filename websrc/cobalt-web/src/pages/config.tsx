import React, { useEffect, useState } from 'react';
import { Card, Button, Form } from 'react-bootstrap';
import { GearFill } from 'react-bootstrap-icons';
import axios from 'axios';

interface ConfigData {
  [key: string]: {
    [key: string]: string;
  };
}

const ConfigManager = () => {
  const [configData, setConfigData] = useState<ConfigData>({});

  // 从后端获取配置数据
  useEffect(() => {
    axios.get('/api/config').then(response => {
      setConfigData(response.data);
    }).catch(error => {
      console.error('获取配置数据出错:', error);
    });
  }, []);

  const handleSetConfig = (category: string, name: string, value: string) => {
    axios.post('/api/config', {
      category,
      name,
      value
    }).then(response => {
      console.log(`设置配置成功: ${response.data}`);
    }).catch(error => {
      console.error(`设置配置失败: ${error}`);
    });
  };

  const handleInputChange = (category: string, name: string, e: React.ChangeEvent<HTMLInputElement>) => {
    const value = e.target.value;
    setConfigData(prevData => ({
      ...prevData,
      [category]: {
        ...prevData[category],
        [name]: value
      }
    }));
  };  

  return (
    <>
      <h1 className="title">配置管理</h1>
      {Object.entries(configData).map(([category, configs]) => (
        <Card key={category} className="mb-3">
          <Card.Header className="d-flex align-items-center justify-content-between">
            {category}
            <GearFill size={24} />
          </Card.Header>
          <Card.Body>
            {Object.entries(configs).map(([name, value]) => (
              <div key={name} className="d-flex align-items-center justify-content-between mb-2">
                <div className="config-name">{name}</div>
                <div className="d-flex align-items-center">
                  <div className="config-value mr-2">{value}</div>
                  <Form.Control
                    type="text"
                    className="config-input mr-2"
                    value={value}
                    onChange={(e) => handleInputChange(category, name, e)}
                  />
                  <Button
                    variant="primary"
                    onClick={() => handleSetConfig(category, name, value)}
                  >
                    Set
                  </Button>
                </div>
              </div>
            ))}
          </Card.Body>
        </Card>
      ))}
    </>
  );
};

export default ConfigManager;

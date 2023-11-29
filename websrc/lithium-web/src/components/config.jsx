import React, { useEffect, useState } from 'react';
import Card from 'react-bootstrap/Card';
import Button from 'react-bootstrap/Button';

const ConfigManager = () => {
    const [configData, setConfigData] = useState(null);

    // 模拟从后端获取配置数据
    useEffect(() => {
        // 这里使用一个定时器模拟异步请求
        const fetchData = setTimeout(() => {
            const data = {
                server: {
                    maxprocess: 10,
                    maxthread: 10
                }
            };
            setConfigData(data);
        }, 1000);

        return () => clearTimeout(fetchData);
    }, []);

    const handleSetConfig = (category, name) => {
        // 在这里处理设置配置的逻辑
        console.log(`Set config for ${category}.${name}`);
    };

    if (!configData) {
        return <div>Loading...</div>;
    }

    return (
        <div>
            {Object.entries(configData).map(([category, configs]) => (
                <Card key={category}>
                    <Card.Header>{category}</Card.Header>
                    <Card.Body>
                        {Object.entries(configs).map(([name, value]) => (
                            <div key={name} className="config-item">
                                <div className="config-name">{name}</div>
                                <div className="config-value">{value}</div>
                                <input type="text" className="config-input" />
                                <Button
                                    variant="primary"
                                    onClick={() => handleSetConfig(category, name)}
                                >
                                    Set
                                </Button>
                            </div>
                        ))}
                    </Card.Body>
                </Card>
            ))}
        </div>
    );
};

export default ConfigManager;

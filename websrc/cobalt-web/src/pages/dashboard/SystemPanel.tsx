import React, { useState, useEffect } from 'react';
import { Button, Card, Accordion, Table } from 'react-bootstrap';
import { BarChartLineFill } from 'react-bootstrap-icons';
import ReactEcharts from 'echarts-for-react';

const CpuUsageChart = ({ testData }) => {
  const [cpuData, setCpuData] = useState(testData.cpuData);

  // 模拟数据变化
  useEffect(() => {
    const intervalId = setInterval(() => {
      const newData = testData.generateData();
      setCpuData(newData.cpuData);
    }, testData.interval);
    return () => clearInterval(intervalId);
  }, [testData]);

  // 渲染折线图
  const renderChart = () => {
    if (!cpuData) {
      return null;
    }

    const options = {
      tooltip: {
        trigger: 'axis',
      },
      xAxis: {
        type: 'category',
        data: cpuData.categories,
      },
      yAxis: {
        type: 'value',
      },
      series: [
        {
          name: 'CPU使用率',
          type: 'line',
          data: cpuData.data,
        },
      ],
    };

    return <ReactEcharts option={options} />;
  };

  return (
    <Card>
      <Card.Header>
        <BarChartLineFill size={16} />
        CPU使用率
      </Card.Header>
      <Card.Body>
        <Button variant="primary" onClick={() => setCpuData(testData.cpuData)}>
          刷新
        </Button>
        {renderChart()}
      </Card.Body>
    </Card>
  );
};

const RamUsageChart = ({ testData }) => {
  const [ramData, setRamData] = useState(testData.ramData);

  // 模拟数据变化
  useEffect(() => {
    const intervalId = setInterval(() => {
      const newData = testData.generateData();
      setRamData(newData.ramData);
    }, testData.interval);
    return () => clearInterval(intervalId);
  }, [testData]);

  // 渲染折线图
  const renderChart = () => {
    if (!ramData) {
      return null;
    }

    const options = {
      tooltip: {
        trigger: 'axis',
      },
      xAxis: {
        type: 'category',
        data: ramData.categories,
      },
      yAxis: {
        type: 'value',
      },
      series: [
        {
          name: '内存使用率',
          type: 'line',
          data: ramData.data,
        },
      ],
    };

    return <ReactEcharts option={options} />;
  };

  return (
    <Card>
      <Card.Header>
        <BarChartLineFill size={16} />
        内存使用率
      </Card.Header>
      <Card.Body>
        <Button variant="primary" onClick={() => setRamData(testData.ramData)}>
          刷新
        </Button>
        {renderChart()}
      </Card.Body>
    </Card>
  );
};

const DiskInfoTable = ({ testData }) => {
  const [diskData, setDiskData] = useState(testData.diskData);

  // 模拟数据变化
  useEffect(() => {
    const intervalId = setInterval(() => {
      const newData = testData.generateData();
      setDiskData(newData.diskData);
    }, testData.interval);
    return () => clearInterval(intervalId);
  }, [testData]);

  // 渲染Accordion组件
  const renderTable = () => {
    if (!diskData) {
      return null;
    }

    return (
      <Accordion defaultActiveKey="0">
        {diskData.map((disk, index) => (
          <Card key={index}>
            <Accordion as={Card.Header} eventKey={index.toString()}>
              {disk.name}
            </Accordion>
            <Accordion.Collapse eventKey={index.toString()}>
              <Card.Body>
                <Table striped bordered hover size="sm">
                  <thead>
                    <tr>
                      <th>类型</th>
                      <th>大小</th>
                      <th>已用空间</th>
                      <th>可用空间</th>
                    </tr>
                  </thead>
                  <tbody>
                    <tr>
                      <td>本地磁盘</td>
                      <td>{disk.size}</td>
                      <td>{disk.used}</td>
                      <td>{disk.available}</td>
                    </tr>
                  </tbody>
                </Table>
              </Card.Body>
            </Accordion.Collapse>
          </Card>
        ))}
      </Accordion>
    );
  };

  return (
    <Card>
      <Card.Header>
        <BarChartLineFill size={16} />
        磁盘信息
      </Card.Header>
      <Card.Body>
        <Button variant="primary" onClick={() => setDiskData(testData.diskData)}>
          刷新
        </Button>
        {renderTable()}
      </Card.Body>
    </Card>
  );
};

const testData = {
  interval: 3000,
  cpuData: {
    categories: ['2024-01-24 14:31:33', '2024-01-24 14:31:36', '2024-01-24 14:31:39', '2024-01-24 14:31:42', '2024-01-24 14:31:45'],
    data: [80, 60, 50, 70, 90],
  },
  ramData: {
    categories: ['2024-01-24 14:31:33', '2024-01-24 14:31:36', '2024-01-24 14:31:39', '2024-01-24 14:31:42', '2024-01-24 14:31:45'],
    data: [30, 40, 50, 60, 70],
  },
  diskData: [
    {
      name: 'C:',
      size: '256GB',
      used: '100GB',
      available: '156GB',
    },
    {
      name: 'D:',
      size: '1TB',
      used: '400GB',
      available: '600GB',
    },
  ],
  generateData: function () {
    const newCpuData = { ...this.cpuData };
    const newRamData = { ...this.ramData };
    const newDiskData = this.diskData.map(disk => ({
      ...disk,
      used: `${parseInt(disk.used) + Math.round(Math.random() * 10)}GB`,
      available: `${parseInt(disk.available) - Math.round(Math.random() * 10)}GB`,
    }));
    newCpuData.categories.push(new Date().toLocaleString());
    newCpuData.data.push(Math.round(Math.random() * 100));
    newRamData.categories.push(new Date().toLocaleString());
    newRamData.data.push(Math.round(Math.random() * 100));
    if (newCpuData.categories.length > 10) {
      newCpuData.categories.shift();
      newCpuData.data.shift();
    }
    if (newRamData.categories.length > 10) {
      newRamData.categories.shift();
      newRamData.data.shift();
    }
    return {
      cpuData: newCpuData,
      ramData: newRamData,
      diskData: newDiskData,
    };
  },
};

const SystemPanel = () => {
  return (
    <>
      <CpuUsageChart testData={testData} />
      <RamUsageChart testData={testData} />
      <DiskInfoTable testData={testData} />
    </>
  );
};

export default SystemPanel;

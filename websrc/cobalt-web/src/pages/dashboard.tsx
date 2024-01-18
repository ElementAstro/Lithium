import React from 'react';
import { Container, Row, Col, Card } from 'react-bootstrap';

const Dashboard = () => {
  return (
    <Container>
      <h2>Dashboard</h2>
      <Row>
        <Col md={6} lg={4}>
          <Card>
            <Card.Body>
              <Card.Title>Card 1</Card.Title>
              <Card.Text>This is card 1.</Card.Text>
            </Card.Body>
          </Card>
        </Col>
        <Col md={6} lg={4}>
          <Card>
            <Card.Body>
              <Card.Title>Card 2</Card.Title>
              <Card.Text>This is card 2.</Card.Text>
            </Card.Body>
          </Card>
        </Col>
        <Col md={6} lg={4}>
          <Card>
            <Card.Body>
              <Card.Title>Card 3</Card.Title>
              <Card.Text>This is card 3.</Card.Text>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default Dashboard;

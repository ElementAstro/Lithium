import React, { useState } from 'react';
import { Container, Form, Button, Row, Col, Card } from 'react-bootstrap';

const SearchPage = () => {
  const [searchTerm, setSearchTerm] = useState('');
  const [searchResults, setSearchResults] = useState([]);

  const handleSearch = (e) => {
    e.preventDefault();
    // 发送搜索请求
    fetch(`https://jsonplaceholder.typicode.com/posts?q=${searchTerm}`)
      .then((response) => response.json())
      .then((data) => setSearchResults(data))
      .catch((error) => console.log(error));
  };

  return (
    <Container>
      <h2>Search Page</h2>
      <Form onSubmit={handleSearch}>
        <Form.Group>
          <Form.Control
            type="text"
            placeholder="Enter search term"
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
          />
        </Form.Group>
        <Button variant="primary" type="submit">
          Search
        </Button>
      </Form>
      <hr />
      {searchResults.length > 0 ? (
        <Row>
          {searchResults.map((result) => (
            <Col key={result.id} md={6} lg={4} className="mb-4">
              <Card>
                <Card.Body>
                  <Card.Title>{result.title}</Card.Title>
                  <Card.Text>{result.body}</Card.Text>
                </Card.Body>
              </Card>
            </Col>
          ))}
        </Row>
      ) : (
        <p>No results found</p>
      )}
    </Container>
  );
};

export default SearchPage;

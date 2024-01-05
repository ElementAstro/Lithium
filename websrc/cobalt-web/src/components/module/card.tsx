import React from "react";
import Accordion from "react-bootstrap/Accordion";
import Card from "react-bootstrap/Card";

function ModuleCard(props) {
  const module = props.module;

  return (
    <Accordion>
      <Card>
        <Card.Header>
          <div className="card-header-container">
            {module.thumbnail && (
              <div className="thumbnail-container">
                <img src={module.thumbnail} alt="thumbnail" />
              </div>
            )}
            <div className="info-container">
              <div className="name">{module.name}</div>
              <div className="author">{module.author}</div>
              <div className="version">{module.version}</div>
              <div className="checkbox">
                <input type="checkbox" />
              </div>
            </div>
          </div>
        </Card.Header>
        <Accordion.Collapse eventKey="0">
          <Card.Body>
            <pre>{JSON.stringify(module.package, null, 2)}</pre>
          </Card.Body>
        </Accordion.Collapse>
      </Card>
    </Accordion>
  );
}

export default ModuleCard;

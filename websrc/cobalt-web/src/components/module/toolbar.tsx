import React from "react";
import Button from "react-bootstrap/Button";

function ToolBar(props) {
  return (
    <div className="toolbar">
      <Button variant="primary" onClick={props.onRefresh}>
        Refresh
      </Button>
      <Button variant="success">Enable</Button>
      <Button variant="danger">Disable</Button>
      <Button variant="outline-danger">Delete</Button>
      <Button variant="primary">Search</Button>
    </div>
  );
}

export default ToolBar;

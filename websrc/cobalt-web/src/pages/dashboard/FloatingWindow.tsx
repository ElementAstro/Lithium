import React, { useState } from "react";
import { Button, Card } from "react-bootstrap";
import {
  ThreeDotsVertical,
  ArrowsMove,
  ArrowsAngleContract,
} from "react-bootstrap-icons";

const FloatingWindow = ({ children }) => {
  const [isOpen, setIsOpen] = useState(false);
  const [position, setPosition] = useState({ x: 0, y: 0 });
  const [dragging, setDragging] = useState(false);
  const [offset, setOffset] = useState({ x: 0, y: 0 });
  const [size, setSize] = useState({
    width: window.innerWidth / 2,
    height: window.innerHeight / 2,
  });

  const handleToggle = () => {
    setIsOpen(!isOpen);
  };

  const handleMouseDown = (e) => {
    setDragging(true);
    setOffset({ x: e.clientX - position.x, y: e.clientY - position.y });
  };

  const handleMouseMove = (e) => {
    if (dragging) {
      setPosition({ x: e.clientX - offset.x, y: e.clientY - offset.y });
    }
  };

  const handleMouseUp = () => {
    setDragging(false);
  };

  const handleResize = () => {
    setSize({ width: window.innerWidth / 2, height: window.innerHeight / 2 });
  };

  return (
    <>
      <Button
        variant="primary"
        onClick={handleToggle}
        className={`toggle-button ${isOpen ? "open" : ""}`}
      >
        <ThreeDotsVertical />
      </Button>
      {isOpen && (
        <Card
          className="floating-window"
          style={{
            top: position.y,
            left: position.x,
            width: size.width,
            height: size.height,
            cursor: dragging ? "grabbing" : "grab",
          }}
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
        >
          <Card.Header>
            <ArrowsMove size={16} className="drag-icon" />
            <ArrowsAngleContract
              size={16}
              className="resize-icon"
              onClick={handleResize}
            />
          </Card.Header>
          <Card.Body>
            <div
              className="content-container"
              style={{ maxHeight: size.height }}
            >
              {children}
            </div>
          </Card.Body>
        </Card>
      )}
    </>
  );
};

export default FloatingWindow;

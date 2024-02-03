import React, { useState, useEffect } from "react";
import { Button, Card } from "react-bootstrap";
import {
  ThreeDotsVertical,
  ArrowsMove,
  ArrowsAngleContract,
} from "react-bootstrap-icons";
import { useSwipeable } from "react-swipeable";
import { Resizable } from "re-resizable";
import styled from "styled-components";

const ToggleButton = styled(Button)`
  position: absolute;
  top: 10px;
  right: 10px;
  z-index: 999;
`;

const FloatingWindowContainer = styled(Card)`
  position: fixed;
  background-color: #ffffff;
  border-radius: 4px;
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.15);
  z-index: 998;
`;

const DragIcon = styled(ArrowsMove)`
  cursor: move;
  margin-right: 8px;
`;

const ResizeIcon = styled(ArrowsAngleContract)`
  cursor: nwse-resize;
  margin-left: auto;
`;

const ContentContainer = styled.div`
  padding: 16px;
`;

const FloatingWindow = ({ children }) => {
  const [isOpen, setIsOpen] = useState(false);
  const [position, setPosition] = useState({ x: 0, y: 0 });
  const [dragging, setDragging] = useState(false);
  const [size, setSize] = useState({
    width: window.innerWidth / 2,
    height: window.innerHeight / 2,
  });

  useEffect(() => {
    const handleResize = () => {
      setSize({ width: window.innerWidth / 2, height: window.innerHeight / 2 });
    };
    window.addEventListener("resize", handleResize);
    return () => {
      window.removeEventListener("resize", handleResize);
    };
  }, []);

  const toggleOpen = () => {
    setIsOpen(!isOpen);
  };

  const handleMouseDown = (e) => {
    setDragging(true);
  };

  const handleMouseMove = (e) => {
    if (dragging) {
      setPosition({ x: e.clientX, y: e.clientY });
    }
  };

  const handleMouseUp = () => {
    setDragging(false);
  };

  const handleResize = (event, direction, ref, delta, position) => {
    setSize({ width: ref.offsetWidth, height: ref.offsetHeight });
  };

  const handlers = useSwipeable({
    onSwipedDown: () => {
      setIsOpen(false);
    },
  });

  return (
    <>
      <ToggleButton
        variant="primary"
        onClick={toggleOpen}
        className={`toggle-button ${isOpen ? "open" : ""}`}
      >
        <ThreeDotsVertical />
      </ToggleButton>
      {isOpen && (
        <FloatingWindowContainer
          style={{
            top: position.y,
            left: position.x,
          }}
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          {...handlers}
        >
          <Card.Header>
            <DragIcon size={16} className="drag-icon" />
            <ResizeIcon size={16} className="resize-icon" />
          </Card.Header>
          <Resizable
            size={{ width: size.width, height: size.height }}
            onResizeStop={handleResize}
            enable={{
              top: false,
              right: true,
              bottom: true,
              left: false,
              topRight: false,
              bottomRight: true,
              bottomLeft: false,
              topLeft: false,
            }}
            minHeight={200}
            minWidth={200}
            maxHeight={window.innerHeight}
            maxWidth={window.innerWidth}
          >
            <Card.Body>
              <ContentContainer>{children}</ContentContainer>
            </Card.Body>
          </Resizable>
        </FloatingWindowContainer>
      )}
    </>
  );
};

export default FloatingWindow;

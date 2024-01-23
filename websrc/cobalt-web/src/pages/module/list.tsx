import React from "react";
import PropTypes from "prop-types";
import { DragDropContext, Droppable, Draggable } from "react-beautiful-dnd";

function ModuleList(props) {
  const { onDragEnd, children } = props;

  return (
    <DragDropContext onDragEnd={onDragEnd}>
      <Droppable droppableId="module-list">
        {(provided) => (
          <div ref={provided.innerRef} {...provided.droppableProps}>
            {React.Children.map(children, (child, index) => (
              <Draggable
                key={child.key}
                draggableId={child.key.toString()}
                index={index}
              >
                {(provided) => (
                  <div
                    ref={provided.innerRef}
                    {...provided.draggableProps}
                    {...provided.dragHandleProps}
                  >
                    {child}
                  </div>
                )}
              </Draggable>
            ))}
            {provided.placeholder}
          </div>
        )}
      </Droppable>
    </DragDropContext>
  );
}

ModuleList.propTypes = {
  onDragEnd: PropTypes.func.isRequired,
  children: PropTypes.node.isRequired,
};

export default ModuleList;

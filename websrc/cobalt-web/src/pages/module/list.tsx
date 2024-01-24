import React from "react";
import PropTypes from "prop-types";
import { DragDropContext, Droppable, Draggable } from "react-beautiful-dnd";

function ModuleList(props: { onDragEnd: any; children: any; draggableClassName: any; }) {
  const { onDragEnd, children, draggableClassName } = props;

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
                    className={draggableClassName}
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
  draggableClassName: PropTypes.string,
};

ModuleList.defaultProps = {
  draggableClassName: "",
};

export default ModuleList;

import React from "react";
import { Button, FormControl, InputGroup } from "react-bootstrap";
import styled, { keyframes } from "styled-components";
import { Circle, Search, ChevronUp, ChevronDown } from "react-bootstrap-icons";

const rotateAnimation = keyframes`
  from {
    transform: rotate(0deg);
  }
  to {
    transform: rotate(360deg);
  }
`;

const StyledToolBar = styled.div`
  margin-bottom: 1rem;
  display: flex;
  align-items: center;

  .search-input {
    flex: 1;
    margin-right: 1rem;
  }

  .refresh-button {
    margin-right: 1rem;

    &.loading {
      svg {
        animation: ${rotateAnimation} 1s linear infinite;
      }
    }
  }

  .sort-select {
    width: 200px;
  }
`;

const ToolBar = (props) => {
  const { onRefresh, onSearchChange, onSortChange } = props;

  const handleRefresh = () => {
    onRefresh();
  };

  const handleSearchChange = (e) => {
    onSearchChange(e.target.value);
  };

  const handleSortChange = (e) => {
    onSortChange(e.target.value);
  };

  return (
    <StyledToolBar>
      <InputGroup className="search-input">
        <FormControl placeholder="Search" onChange={handleSearchChange} />
        <InputGroup.Text>
          <Search size={16} />
        </InputGroup.Text>
      </InputGroup>
      <Button
        variant="outline-secondary refresh-button"
        onClick={handleRefresh}
        className={props.loading ? "loading" : ""}
      >
        {props.loading ? (
          <Circle size={16} />
        ) : (
          <Circle size={16} className="mr-1" />
        )}
        Refresh
      </Button>
      <select className="form-select sort-select" onChange={handleSortChange}>
        <option value="name.asc">Name (A-Z)</option>
        <option value="name.desc">Name (Z-A)</option>
        <option value="author.asc">Author (A-Z)</option>
        <option value="author.desc">Author (Z-A)</option>
        <option value="version.asc">Version (Low to High)</option>
        <option value="version.desc">Version (High to Low)</option>
      </select>
    </StyledToolBar>
  );
};

export default ToolBar;

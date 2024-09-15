import pytest
from ..tools.html import Tag


@pytest.fixture
def sample_tag():
    return Tag(name="div", attrs={"class": "container"}, contents=["Hello, World!"])


def test_tag_initialization(sample_tag):
    assert sample_tag.name == "div"
    assert sample_tag.attributes == {"class": "container"}
    assert sample_tag.contents == ["Hello, World!"]
    assert not sample_tag.is_single


def test_set_attr(sample_tag):
    sample_tag.set_attr("id", "main")
    assert sample_tag.attributes["id"] == "main"


def test_remove_attr(sample_tag):
    sample_tag.set_attr("id", "main")
    sample_tag.remove_attr("id")
    assert "id" not in sample_tag.attributes


def test_add_content(sample_tag):
    sample_tag.add_content("New content")
    assert "New content" in sample_tag.contents


def test_remove_content(sample_tag):
    sample_tag.add_content("New content")
    sample_tag.remove_content("New content")
    assert "New content" not in sample_tag.contents


def test_find_by_tag():
    parent_tag = Tag(name="div")
    child_tag = Tag(name="p")
    parent_tag.add_content(child_tag)
    found_tags = parent_tag.find_by_tag("p")
    assert len(found_tags) == 1
    assert found_tags[0] == child_tag


def test_render(sample_tag):
    rendered_html = sample_tag.render()
    assert rendered_html == '<div class="container">Hello, World!</div>'


def test_render_single_tag():
    single_tag = Tag(name="br", is_single=True)
    rendered_html = single_tag.render()
    assert rendered_html == '<br />'


def test_render_with_context():
    tag_with_template = Tag(name="p", contents=["Hello, {{name}}!"])
    rendered_html = tag_with_template.render(context={"name": "Alice"})
    assert rendered_html == '<p>Hello, Alice!</p>'


def test_str_method(sample_tag):
    assert str(sample_tag) == sample_tag.render()

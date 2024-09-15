import json
import re
import argparse


class Tag:
    def __init__(self, name, is_single=False, attrs=None, contents=None):
        """
        Initialize a new HTML tag.

        Args:
            name (str): The name of the tag (e.g., 'div', 'p').
            is_single (bool): Whether the tag is a self-closing tag (e.g., <br />).
            attrs (dict): A dictionary of attributes for the tag (e.g., {'class': 'my-class'}).
            contents (list): A list of contents inside the tag (e.g., other Tag objects or strings).
        """
        self.name = name
        self.contents = contents if contents else []
        self.attributes = attrs if attrs else {}
        self.is_single = is_single

    def set_attr(self, key, value):
        """
        Set an attribute for the tag.

        Args:
            key (str): The attribute name.
            value (str): The attribute value.
        """
        self.attributes[key] = value

    def remove_attr(self, key):
        """
        Remove an attribute from the tag.

        Args:
            key (str): The attribute name to remove.
        """
        if key in self.attributes:
            del self.attributes[key]

    def add_content(self, content):
        """
        Add content inside the tag.

        Args:
            content (Tag or str): The content to add.
        """
        if not self.is_single:
            self.contents.append(content)

    def remove_content(self, content):
        """
        Remove content from inside the tag.

        Args:
            content (Tag or str): The content to remove.
        """
        if content in self.contents:
            self.contents.remove(content)

    def find_by_tag(self, tag_name):
        """
        Find all contents by tag name.

        Args:
            tag_name (str): The name of the tag to find.

        Returns:
            list: A list of Tag objects with the specified tag name.
        """
        found = []
        for content in self.contents:
            if isinstance(content, Tag):
                if content.name == tag_name:
                    found.append(content)
                found.extend(content.find_by_tag(tag_name))
        return found

    def render(self, context=None):
        """
        Render the tag as an HTML string.

        Args:
            context (dict): A dictionary for template rendering.

        Returns:
            str: The rendered HTML string.
        """
        attrs = ''.join(f' {attr}="{value}"' for attr,
                        value in self.attributes.items())
        inner_html = ''.join(self.render_str(c, context)
                             for c in self.contents)
        if self.is_single:
            return f'<{self.name}{attrs} />'
        else:
            return f'<{self.name}{attrs}>{inner_html}</{self.name}>'

    def render_str(self, text, context):
        """
        Render a string with context for template rendering.

        Args:
            text (str): The text to render.
            context (dict): A dictionary for template rendering.

        Returns:
            str: The rendered string.
        """
        if context is not None and isinstance(text, str):
            return re.sub(r'\{\{(\w+)\}\}', lambda m: str(context.get(m.group(1), m.group(0))), text)
        return text

    def __str__(self):
        """
        Convert the tag to a string by rendering it.

        Returns:
            str: The rendered HTML string.
        """
        return self.render()


class HTMLDocument:
    def __init__(self, title=""):
        """
        Initialize a new HTML document.

        Args:
            title (str): The title of the document.
        """
        self.root = Tag("html")
        self.head = Tag("head")
        self.body = Tag("body")
        self.root.add_content(self.head)
        self.root.add_content(self.body)
        self.bootstrap_enabled = False

        if title:
            self.set_title(title)

    def enable_bootstrap(self, version="5.2.3"):
        """
        Enable Bootstrap in the document.

        Args:
            version (str): The version of Bootstrap to use.
        """
        if not self.bootstrap_enabled:
            link_tag = Tag("link", is_single=True, attrs={
                "rel": "stylesheet",
                "href": f"https://stackpath.bootstrapcdn.com/bootstrap/{version}/css/bootstrap.min.css"
            })
            self.head.add_content(link_tag)

            script_tag_js = Tag("script", attrs={
                "src": f"https://stackpath.bootstrapcdn.com/bootstrap/{version}/js/bootstrap.bundle.min.js"
            })
            self.body.add_content(script_tag_js)

            self.bootstrap_enabled = True

    def set_title(self, title):
        """
        Set the title of the document.

        Args:
            title (str): The title of the document.
        """
        title_tag = Tag("title")
        title_tag.add_content(title)
        self.head.add_content(title_tag)

    def add_tag(self, tag, to="body"):
        """
        Add a tag to the document.

        Args:
            tag (Tag): The tag to add.
            to (str): The section to add the tag to ('head' or 'body').
        """
        if to == "head":
            self.head.add_content(tag)
        else:
            self.body.add_content(tag)

    def render(self, context=None):
        """
        Render the document as an HTML string.

        Args:
            context (dict): A dictionary for template rendering.

        Returns:
            str: The rendered HTML string.
        """
        doctype = "<!DOCTYPE html>\n"
        return doctype + self.root.render(context)

    def __str__(self):
        """
        Convert the document to a string by rendering it.

        Returns:
            str: The rendered HTML string.
        """
        return self.render()


def generate_html_from_json(json_data, context=None):
    """
    Generate HTML from a JSON structure.

    Args:
        json_data (dict or str): The JSON data representing the HTML structure.
        context (dict): A dictionary for template rendering.

    Returns:
        Tag or str: The generated HTML tag or string.
    """
    if isinstance(json_data, dict):
        tag_name = json_data.get("tag", "div")
        attrs = json_data.get("attrs", {})
        contents = json_data.get("contents", [])
        is_single = json_data.get("is_single", False)

        tag = Tag(tag_name, is_single, attrs)

        if isinstance(contents, list):
            for content in contents:
                tag.add_content(generate_html_from_json(content, context))
        elif isinstance(contents, str):
            tag.add_content(contents.format(**(context if context else {})))
        return tag
    elif isinstance(json_data, str):
        return json_data.format(**(context if context else {}))
    return ""


def main():
    """
    Main function to parse command line arguments and generate HTML document.
    """
    parser = argparse.ArgumentParser(
        description="Generate HTML from JSON structure.")
    parser.add_argument(
        "json_file", help="Path to the JSON file containing the HTML structure.")
    parser.add_argument(
        "--context", help="Path to the JSON file containing the context for template rendering.", default=None)
    parser.add_argument(
        "--output", help="Path to the output HTML file.", default="output.html")
    parser.add_argument(
        "--title", help="Title of the HTML document.", default="Generated Page")
    parser.add_argument(
        "--enable-bootstrap", help="Enable Bootstrap in the HTML document.", action="store_true")

    args = parser.parse_args()

    with open(args.json_file, 'r') as f:
        json_data = json.load(f)

    context = None
    if args.context:
        with open(args.context, 'r') as f:
            context = json.load(f)

    doc = HTMLDocument(args.title)
    if args.enable_bootstrap:
        doc.enable_bootstrap()

    generated_html = generate_html_from_json(json_data, context)
    doc.add_tag(generated_html)

    with open(args.output, 'w') as f:
        f.write(str(doc))

    print(f"HTML document generated and saved to {args.output}")


if __name__ == "__main__":
    main()

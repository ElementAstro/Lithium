import json
import re

class Tag:
    def __init__(self, name, is_single=False, attrs=None, contents=None):
        self.name = name
        self.contents = contents if contents else []
        self.attributes = attrs if attrs else {}
        self.is_single = is_single

    def set_attr(self, key, value):
        self.attributes[key] = value

    def add_content(self, content):
        if not self.is_single:
            self.contents.append(content)

    def render(self, context=None):
        attrs = ''.join(f' {attr}="{value}"' for attr, value in self.attributes.items())
        inner_html = ''.join(self.render_str(c, context) for c in self.contents)
        if self.is_single:
            return f'<{self.name}{attrs} />'
        else:
            return f'<{self.name}{attrs}>{inner_html}</{self.name}>'

    def render_str(self, text, context):
        if context is not None and isinstance(text, str):
            return re.sub(r'\{\{(\w+)\}\}', lambda m: str(context.get(m.group(1), m.group(0))), text)
        return text

    def __str__(self):
        return self.render()

class HTMLDocument:
    def __init__(self, title=""):
        self.root = Tag("html")
        self.head = Tag("head")
        self.body = Tag("body")
        self.root.add_content(self.head)
        self.root.add_content(self.body)
        self.bootstrap_enabled = False

        if title:
            self.set_title(title)

    def enable_bootstrap(self, version="5.2.3"):
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
        title_tag = Tag("title")
        title_tag.add_content(title)
        self.head.add_content(title_tag)

    def add_tag(self, tag, to="body"):
        if to == "head":
            self.head.add_content(tag)
        else:
            self.body.add_content(tag)

    def render(self, context=None):
        doctype = "<!DOCTYPE html>\n"
        return doctype + self.root.render(context)

    def __str__(self):
        return self.render()

def generate_html_from_json(json_data, context=None):
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

# Example usage
if __name__ == "__main__":
    doc = HTMLDocument("Generated Page")
    doc.enable_bootstrap()

    # Example JSON structure
    json_structure = {
        "tag": "div",
        "attrs": {"class": "container"},
        "contents": [
            {
                "tag": "h1",
                "contents": "Welcome, {{user}}!"
            },
            {
                "tag": "p",
                "contents": "This paragraph is a demonstration of JSON to HTML."
            },
            {
                "tag": "div",
                "attrs": {"class": "card"},
                "contents": [
                    {
                        "tag": "div",
                        "attrs": {"class": "card-body"},
                        "contents": [
                            {
                                "tag": "h5",
                                "attrs": {"class": "card-title"},
                                "contents": "Card Title"
                            },
                            {
                                "tag": "p",
                                "attrs": {"class": "card-text"},
                                "contents": "Some example text."
                            },
                            {
                                "tag": "a",
                                "attrs": {"href": "#", "class": "btn btn-primary"},
                                "contents": "Go somewhere"
                            }
                        ]
                    }
                ]
            }
        ]
    }

    # Generate HTML from JSON
    generated_html = generate_html_from_json(json_structure, {"user": "John Doe"})
    doc.add_tag(generated_html)

    print(doc)

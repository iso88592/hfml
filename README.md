# 🔋 HFML
*The future of web*

HFML is an easy to read, easy to parse HTML alternative that powers the World Wide Web with high performance. We show an alternative how the Web should have been designed from the beginning. We do not wish to maintain any compatibility with HTML, but we also incorporate good web development design ideas.

## Capabilities

 * **Simple Syntax:**

    HFML uses a minimalistic and human-readable syntax where content is always the focus. It avoids the complexity of traditional HTML tags and excessive nesting.

 * **Flexible Metadata:**
    
    Metadata within square brackets allows you to modify the behavior, structure, and appearance of the content without complicating the syntax.

 * **Performance-Oriented:**
    
    By removing redundant and legacy structures from traditional HTML, HFML provides a streamlined, fast experience both for developers and users.
 
 * **Built-In Interactivity:**
    
    HFML allows you to easily define actions (like clicks or hover events) directly within the markup, without the need for additional JavaScript frameworks or other means of Remote Code Execution.
 
 * **Cross-Platform:**
    
    HFML can be parsed and rendered across different platforms and devices, providing a lightweight solution for modern web development.

 * **Easily Extendable:**
    
    HFML can be customized to support new tags, behaviors, or integrations, making it an excellent choice for projects of all sizes.

## How it works
HFML is built around the concept of content-first design. At its core, the format simplifies how content is expressed and interacted with on the web. Here's how it works:
 1. Content as the Primary Focus: HFML's syntax allows the content itself to be front-and-center. Components are defined in `<>` and is a recursive type, just like in HTML. You can place text content in `{}`, with metadata inside square brackets `[]` controlling how that content is rendered or behaves. For example, `<{hello world}>` just outputs "hello world" on the page.

 2. Plain text is always inside `{}`, so it's easy to parse and highlight. All HFML texts must be well-formed, and everything inside `{}` are considered to be text.

 3. Simple Metadata Modifiers: Metadata is attached directly to the content using square brackets `[]`, specifying things like styles, actions, and behavior. For example:

```<{hello world}[input:button][click:alert({hello})]>```

In this example:

 * `[input:button]` turns the content into a button.
 * `[click:alert({hello})]` triggers an alert when the button is clicked.

 3. Nesting for Complex Layouts: While the default is simple, HFML also supports nesting content. Nested elements are still treated as components, and metadata can be applied to them, providing a clean and simple way to handle complex structures:
    ```hfml
    <[div][style:flex] 
        <[header][style:text-align=center]{Welcome!}> 
        <[content][style:padding=10px]{This is your content.}>
    >
    ```
Here, the outer div has a flex layout, and each nested element (header and content) has its own style applied.

 4. Efficient Parsing: HFML can be parsed quickly and easily by browsers or any custom parser. The format avoids unnecessary legacy structures and instead focuses on streamlined, high-performance rendering.

## Why HFML?

HFML is a modern alternative to HTML that prioritizes **readability**, **performance**, and **flexibility**. It's designed for developers who want a more intuitive way to structure content without the bloat and complexity of traditional web technologies. By focusing on the content itself and using metadata to adjust its behavior and appearance, HFML provides a cleaner, more efficient approach to web development.

 * Readable: HFML’s syntax is minimal and human-friendly, designed to be intuitive and simple to work with.
 * Modular: With its ability to include and modify content, HFML is easily extensible for new features, components, and actions.
 * Future-Proof: Built with modern web requirements in mind, HFML delivers the performance and scalability needed for next-generation web development.

## Getting Started

 1. **Installation**

    To start using HFML, you’ll need to install a parser that converts HFML into HTML (or another format of your choice). You can easily run your own instance of the HFML-to-HTML API, or use the open-source implementation available on DockerHub.

 2. **Writing HFML**

    Writing HFML is simple:
        Use the `<...>` syntax for components.
        Use `{}` to include text.
        Use `[<metadata>]` to control how content behaves or appears.
        Nest components and apply actions as needed for more complex layouts.

    Example:
```
    <{hello world}[input:button][click:alert({You clicked!})]>
```
  3. **Running the Parser**

     To render your HFML content, pass it through the parser (either local or online). The parser converts HFML to standard HTML, which can be displayed in any web browser.

  4. **Running the Browser**
    
     Alternatively, if you wish to directly render HFML documents, try out the original browser! Just build it yourself here, or get the latest version built in our CI!

## Examples
For more examples, please refer to the Live Demo inside *js* and the hfml files inside *examples*.

## Future Features

 * **Native Browser Support:** We aim to integrate native browser support for HFML to simplify usage and adoption.
 * **Enhanced Metadata:** New modifiers and actions to control more detailed interactions and layouts.
 * **Community-Driven Extensions:** HFML will grow with contributions from the community, allowing new features, elements, and standards to emerge organically.
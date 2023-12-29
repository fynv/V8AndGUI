let input_text = new InputText("##URL", 256, "https://www.gutenberg.org/cache/epub/1065/pg1065.txt");

let button_get = new Button("Get!");
button_get.onClick = async () => {
    let url = input_text.text;
    let text = await http.getAsync(url, true);
    print(`Text From: ${url}`);
    print(text);
};

scriptWindow.add(new Text("url:"));
scriptWindow.add(new SameLine());
scriptWindow.add(input_text);
scriptWindow.add(button_get);
scriptWindow.show = true;

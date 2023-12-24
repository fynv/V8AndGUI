(async ()=>
{
    text = await http.getAsync("https://www.gutenberg.org/cache/epub/1065/pg1065.txt", true);
    print(text);        
    
})()

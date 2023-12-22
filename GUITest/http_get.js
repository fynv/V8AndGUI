function httpGetAsync(url, is_text) {
    return new Promise((resolve, reject) => {
        http.getAsync(url, is_text, (suc, data) => {
            if (suc) {
                resolve(data);
            }
            else {
                reject();
            }
        });
    });
}

(async ()=>
{
    text = await httpGetAsync("https://www.gutenberg.org/cache/epub/1065/pg1065.txt", true);
    print(text);        
    
})()

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
    text = await httpGetAsync("http://www.baidu.com", true);
    print(text);        
    
})()

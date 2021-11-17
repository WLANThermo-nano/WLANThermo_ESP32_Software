## 
The project is develop using nodejs version _v12.15.0_ and yarn version _1.22.0_

## Project setup
```
yarn install
```

### Compiles and hot-reloads for development
```
yarn serve
```

the mock APIs are located at `webui\src\demo\mocks\data.js`.
The APIs logs are write to console.


### Compiles and minifies for production
The build result is at _dist_ folder, 
```
// build for all products
yarn build

// build individual project
yarn build-nano
yarn build-mini
yarn build-link
yarn build-bone
yarn build-cloud
```

### Lints and fixes files
```
yarn lint
```

### Customize configuration
See [Configuration Reference](https://cli.vuejs.org/config/).

The following command is useful when you want to check the current configs.
```
vue inspect > inspect-config.js
```


### Working with icon font
The icons are located at _webui\src\icons\selection.json_
The icons can be imported and viewing with [icon moon](https://icomoon.io/app/#/select)
You can add/remove icons and download the result.

The download file structur is like bellow
```
|demo.html
|style.css
|selection.json
├───demo-files
└───fonts
```

By opening the `demo.html` you can find the icon class you can use.

copy `style.css` and `fonts` folder to the path _webui\src\assets\icons_

In `style.css` file, change the following, note that you have to add `~@/assets/icons` to the path.
If the bundle size is not a problem for you, you can keep other font files. 
```css
// from
  src:  url('fonts/icomoon.eot?anupur');
  src:  url('fonts/icomoon.eot?anupur#iefix') format('embedded-opentype'),
    url('fonts/icomoon.ttf?anupur') format('truetype'),
    url('fonts/icomoon.woff?anupur') format('woff'),
    url('fonts/icomoon.svg?anupur#icomoon') format('svg');


// to 
src:  url('~@/assets/icons/fonts/icomoon.ttf?anupur') format('truetype');
```

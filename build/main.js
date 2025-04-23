const imports = {
    js: {
      log: (value) => {
        console.log("WASM says:", value);
      },
      createElement: (type,id) => {
        let elementName = "";

        for (let i = ptr; memBuffer[i] !== 0; i++) {
            elementName += String.fromCharCode(memBuffer[i]);
        }

        try {
            const newElement = document.createElement(elementName);
            newElement.textContent = `This is a <${elementName}> element from WASM!`;
            mainElement.appendChild(newElement);
        } catch (e) {
            console.error(`Invalid element name from WASM: "${elementName}"`);
        }
      },
      setElementText: (id, text) => {
        const elm = document.getElementById(id);
        elm.textContent = text;
      },
      changeElement: (id,parentID,newType) => {
        const newElement = document.createElement(newTagName);
        for (let attr of oldElement.attributes) {
            newElement.setAttribute(attr.name, attr.value);
        }
        newElement.innerHTML = oldElement.innerHTML;
        oldElement.replaceWith(newElement);
        return newElement;
      },
    }


};
  
WebAssembly.instantiateStreaming(fetch('fibertree.wasm'), imports)
.then(obj => {
    obj.instance.exports.run();
});

let memory;
let get_comp;

async function loadWasm() {
    const response = await WebAssembly.instantiateStreaming(fetch('fibertree.wasm'));
    const exports = response.instance.exports;
    memory = exports.memory;
    get_comp = exports.get_comp;
}

// Call loadWasm immediately
loadWasm();

document.addEventListener("DOMContentLoaded", function () {
    const mainElement = document.getElementById("main");
    // Add dynamic component when 'Add Component' is clicked
    const addComp = document.getElementById("addcomp");
    addComp.addEventListener('click', () => {
        const memBuffer = new Uint8Array(memory.buffer);
        const ptr = get_comp(); // returns pointer to null-terminated string
        let elementName = "";

        for (let i = ptr; memBuffer[i] !== 0; i++) {
            elementName += String.fromCharCode(memBuffer[i]);
        }

        try {
            const newElement = document.createElement(elementName);
            newElement.textContent = `This is a <${elementName}> element from WASM!`;
            mainElement.appendChild(newElement);
        } catch (e) {
            console.error(`Invalid element name from WASM: "${elementName}"`);
        }
    });
});
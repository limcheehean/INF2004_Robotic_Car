import './App.css';
import {useState} from "react";
import TextArea from "antd/es/input/TextArea";

const App = () => {

    const [data, setData] = useState("");

    const get_maze_map = () => {
        try {
            const map = JSON.parse(data);
            const width = map.reduce((a, b) => a.x > b.x ? a.x: b.x) + 1;
            const height = map.reduce((a, b) => a.y > b.y ? a.y: b.y) + 1;
            const elements = [];

            for (let row = 0; row < height; row++) {
                let cells = [];
                for (let col = 0; col < width; col++) {
                    const block = map.find(m => m.x === col && m.y === row);
                    const classes = [];
                    block.neighbours.forEach(neighbour => {
                        if (neighbour.x === block.x + 1)
                            classes.push("right");
                        if (neighbour.x === block.x - 1)
                            classes.push("left");
                        if (neighbour.y === block.y - 1)
                            classes.push("top");
                        if (neighbour.y === block.y + 1)
                            classes.push("bottom");

                    })
                    cells.push(<div className={"cell " + classes.join(" ") + (block.shortest_path ? " shortest": "")}><p>({col}, {row})</p></div>);
                }
                elements.push(<div className="row">
                    {cells}
                </div>)
            }
            return elements;
        } catch (e) {
            console.log(e);
            return <div></div>
        }
    }

    return <div className="App">

        <p>Please paste your maze output here:</p>
        <TextArea rows={6} value={data}  onChange={e => setData(e.target.value)}/>
        <div className="map">
            {get_maze_map()}
        </div>
    </div>

}

export default App;

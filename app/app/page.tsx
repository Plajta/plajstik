import { JoystickModel } from "~/components/joystick-model";
import { Canvas } from "~/components/canvas";

export default function Home() {
    return (
        <div className="h-[90vh] flex justify-center items-center">
            <Canvas>
                <JoystickModel />
            </Canvas>
        </div>
    );
}

"use client"

import { Button } from "~/components/ui/button";
import { Box, Code } from "lucide-react";

export type View = "model" | "editor"

interface ViewSwitcherProps {
    view: View
    setView: (view: View) => void
}

export function ViewSwitcher({view, setView}: ViewSwitcherProps) {
    return (
        <Button variant="ghost" size="icon"
                onClick={() => setView(view === "editor" ? "model" : "editor")}>
            {view === "editor" ? <Code /> : <Box />}
        </Button>
    )
}
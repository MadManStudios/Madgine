import Engine

async def SubAsync(Widget):
	return await Widget.PointerEnter

@Engine.Behavior
async def TestBehavior(input : str, Widget : Engine.Contextual[Engine.Widgets.WidgetBase]):
	test = await SubAsync(Widget)	
	return test
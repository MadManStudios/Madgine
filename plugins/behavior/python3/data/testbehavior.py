import Engine

from dump import dump

async def SubAsync(Widget):
	print(Widget.PointerEnter)
	return await Widget.PointerEnter

@Engine.Behavior
async def TestBehavior(input : str, Widget : Engine.Named[Engine.Widgets.WidgetBase]):
	test = await SubAsync(Widget)
	print(type(Widget))
	print(Widget)
	return test
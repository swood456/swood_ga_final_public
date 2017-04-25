function update (component, frame_params)
	if frame_params_get_input_left(frame_params) then
		entity = component_get_entity(component)
		entity_translate(entity, -0.1, 0.0, 0.0)
	end
	if frame_params_get_input_right(frame_params) then
		entity = component_get_entity(component)
		entity_translate(entity, 0.1, 0.0, 0.0)
	end
	if frame_params_get_input_down(frame_params) then
		entity = component_get_entity(component)
		entity_translate(entity, 0.0, 0.0, 0.1)
	end
	if frame_params_get_input_up(frame_params) then
		entity = component_get_entity(component)
		entity_translate(entity, 0.0, 0.0, -0.1)
	end
	if frame_params_get_input_rot_left(frame_params) then
		entity = component_get_entity(component)
		entity_rotate(entity, 0.075)
	end
	if frame_params_get_input_rot_right(frame_params) then
		entity = component_get_entity(component)
		entity_rotate(entity, -0.075)
	end
end
